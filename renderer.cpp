#include "renderer.h"

#include "delayline.h"
#include "differentiator.h"
#include "movingaverage.h"

#define TIME_RENDER_FUNC
#ifdef TIME_RENDER_FUNC
#include "functimer.h"
// #define SHOW_AVG_RENDER_TIME
#define SHOW_PANIC
#endif

#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QVector>
#include <QPixmap>

#include <cmath>


Renderer::Renderer(QObject *parent)
	: QObject{parent}
{

}

void Renderer::calcScaling()
{
	if(pixmap != nullptr) {
		w = pixmap->width();
		h = pixmap->height();
		cx = 0.5 * w;
		cy = 0.5 * h;

		plotBuffer.reserve(4 * timeLimit_ms * audioFramesPerMs);
		sweepParameters.setWidthFrameRate(w, audioFramesPerMs);
	}
}


void Renderer::render(const QVector<QVector<float>> &inputBuffers, int64_t framesAvailable, int64_t currentFrame)
{
	bool panicMode = false;

#ifdef TIME_RENDER_FUNC
	constexpr size_t historyLength = 10;
	static MovingAverage<double, historyLength> movingAverage;
	static double renderTime = 0.0;

	[[maybe_unused]] static int64_t callCount = 0;
	[[maybe_unused]] static double total_renderTime = 0.0;

	// collect data for overall average
	total_renderTime += renderTime;
	callCount++;

	const double mov_avg_renderTime = movingAverage.get(renderTime);
	panicMode = (mov_avg_renderTime > timeLimit_ms);

#ifdef SHOW_PANIC
	if(panicMode) {
		qDebug() << QStringLiteral("Panic: recent avg (%1ms) > plot-interval (%2ms)")
					.arg(mov_avg_renderTime, 0, 'f', 2)
					.arg(timeLimit_ms);
	}
#endif

#ifdef SHOW_AVG_RENDER_TIME
	if(callCount % 100 == 0) {
		qDebug() << QStringLiteral("Avg render time: Overall=%1ms Recent(last %2)=%3ms")
					.arg(total_renderTime / std::max(1ll, callCount), 0, 'f', 2)
					.arg(historyLength)
					.arg(mov_avg_renderTime, 0, 'f', 2);
	}
#endif

	FuncTimer<std::chrono::milliseconds> funcTimer(&renderTime);

#endif // TIME_RENDER_FUNC

	constexpr bool catchAllFrames = false;
	const bool drawLines =  ( sweepParameters.connectDots &&
							  plotMode == Sweep  &&
							  !panicMode &&
							  (sweepParameters.getSamplesPerSweep() > 25)
							  );

	QPainter painter(pixmap);
	painter.beginNativePainting();
	painter.setCompositionMode(compositionMode);
	painter.setRenderHint(QPainter::TextAntialiasing, false);

	if(--darkenCooldownCounter == 0) {
		// darken:
		painter.setBackgroundMode(Qt::OpaqueMode);
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.fillRect(pixmap->rect(), darkencolor);
		darkenCooldownCounter = darkenNthFrame;
	}

	// set pen
	painter.setRenderHint(QPainter::Antialiasing, !panicMode);
	const QPen pen{phosphorColor, beamWidth, Qt::SolidLine,
				panicMode ? Qt::SquareCap : Qt::RoundCap,
				Qt::BevelJoin};
	painter.setPen(pen);

	// todo: whenever upsampling changes, reset this with upsampled value
	int64_t expected = expectedFrames * sweepParameters.upsampleFactor;
	int64_t firstFrameToPlot = catchAllFrames ? 0ll : std::max<int64_t>(0ll, framesAvailable - 2 * expected);

	// draw
	for(int64_t i = firstFrameToPlot; i < framesAvailable; i++) {

		// types converted here : audio data is float, graphics is qreal (aka double)
		double ch0val = static_cast<double>(inputBuffers[0][i]);
		double ch1val = (numInputChannels > 1 ? static_cast<double>(inputBuffers[1][i]) : 0.0);
		// ---

		switch(plotMode) {
		case XY:
		default:
			plotBuffer.append({(1.0 + ch0val) * cx, (1.0 - ch1val) * cy});
			break;
		case MidSide:
		{
			constexpr double rsqrt2 = 0.707;
			plotBuffer.append({(1.0 + rsqrt2 * (ch0val - ch1val)) * cx,
							   (1.0 - rsqrt2 * (ch0val + ch1val)) * cy});
		}
			break;
		case Sweep:
		{
			static Differentiator<double> d;
			static DelayLine<double, d.delayTime> delayLine;
			static bool triggered = false;
			static qreal x = 0.0;
			static QPointF lastPoint{0.0, cy * (1.0 - sweepParameters.triggerLevel)};
			const double &source = ch0val;
			double slope = d.get(source) * sweepParameters.slope;
			double delayed = delayLine.get(source);

			triggered = triggered
						|| !sweepParameters.triggerEnabled // when trigger disabled -> Always Triggered
						|| (sweepParameters.triggerMin <= delayed && delayed <= sweepParameters.triggerMax && slope > 0.0);

			if(triggered) {
				QPointF pt{x, cy * (1.0 - delayed)};
				if(drawLines)  {
					plotBuffer.append(lastPoint);
				}
				lastPoint = pt;
				plotBuffer.append(pt);
				x += sweepParameters.sweepAdvance;
				if(x > w) { // sweep completed
					x = 0.0;
					triggered = false;
					lastPoint = {x, cy * (1.0 - sweepParameters.triggerLevel)};
				}
			}
		}
			break;

		} // ends switch
	} // ends loop over i

	if(drawLines) {
		painter.drawLines(plotBuffer);
	} else {
		painter.drawPoints(plotBuffer);
	}
	plotBuffer.clear();

	constexpr bool debugPlotBufferSize = false;
	if constexpr(debugPlotBufferSize) {
		static decltype(plotBuffer.size()) maxSize = 0ll;
		if(plotBuffer.size() > maxSize) {
			maxSize = plotBuffer.size();
			qDebug() << "new size:" << maxSize;
		}
	}

	if(showTrigger) {
		drawTrigger(&painter);
	}

	painter.endNativePainting();
	freshRender = true;
	emit renderedFrame(currentFrame);
}

void Renderer::drawTrigger(QPainter* painter)
{
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
	const QPen pen{QColor{128, 32, 32, 192}, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin};
	const QBrush brush{QColor{64, 16, 16, 112}};
	painter->setBrush(brush);
	painter->setPen(pen);
	double yMax = cy * (1.0 - sweepParameters.triggerMax);
	double y = cy * (1.0 - sweepParameters.triggerLevel);
	double yMin = cy * (1.0 - sweepParameters.triggerMin);
	QRectF rect{QPointF{0, yMax}, QPointF{cx * 2, yMin}};
	painter->drawRect(rect);
	painter->drawLine(QPointF{0, y}, QPointF{cx * 2, y});
}

bool Renderer::getShowTrigger() const
{
	return showTrigger;
}

void Renderer::setShowTrigger(bool newShowTrigger)
{
	showTrigger = newShowTrigger;
}



SweepParameters Renderer::getSweepParameters() const
{
	return sweepParameters;
}

void Renderer::setSweepParameters(const SweepParameters &newSweepParameters)
{
	sweepParameters = newSweepParameters;
	calcScaling();
}

double Renderer::getTimeLimit_ms() const
{
	return timeLimit_ms;
}

void Renderer::setTimeLimit_ms(double newTimeLimit_ms)
{
	timeLimit_ms = newTimeLimit_ms;
}

bool Renderer::getFreshRender() const
{
	return freshRender;
}

void Renderer::setFreshRender(bool newFreshRender)
{
	freshRender = newFreshRender;
}

QPixmap *Renderer::getPixmap() const
{
	return pixmap;
}

void Renderer::setPixmap(QPixmap *newPixmap)
{
	pixmap = newPixmap;
}

int Renderer::getAudioFramesPerMs() const
{
	return audioFramesPerMs;
}

void Renderer::setAudioFramesPerMs(int newAudioFramesPerMs)
{
	audioFramesPerMs = newAudioFramesPerMs;
}

QColor Renderer::getDarkencolor() const
{
	return darkencolor;
}

void Renderer::setDarkencolor(const QColor &newDarkencolor)
{
	darkencolor = newDarkencolor;
}

int Renderer::getDarkenNthFrame() const
{
	return darkenNthFrame;
}

void Renderer::setDarkenNthFrame(int newDarkenNthFrame)
{
	darkenNthFrame = newDarkenNthFrame;
	darkenCooldownCounter = darkenNthFrame;
}

int64_t Renderer::getExpectedFrames() const
{
	return expectedFrames;
}

void Renderer::setExpectedFrames(int64_t newExpectedFrames)
{
	expectedFrames = newExpectedFrames;
}

qreal Renderer::getBeamWidth() const
{
	return beamWidth;
}

void Renderer::setBeamWidth(qreal newBeamWidth)
{
	beamWidth = newBeamWidth;
}

qreal Renderer::getBeamIntensity() const
{
	return beamIntensity;
}

void Renderer::setBeamIntensity(qreal newBeamIntensity)
{
	beamIntensity = newBeamIntensity;
}

int Renderer::getNumInputChannels() const
{
	return numInputChannels;
}

void Renderer::setNumInputChannels(int newNumInputChannels)
{
	numInputChannels = newNumInputChannels;
}

QColor Renderer::getPhosphorColor() const
{
	return phosphorColor;
}

void Renderer::setPhosphorColor(const QColor &newPhosphorColor)
{
	phosphorColor = newPhosphorColor;
}

QPainter::CompositionMode Renderer::getCompositionMode() const
{
	return compositionMode;
}

void Renderer::setCompositionMode(QPainter::CompositionMode newCompositionMode)
{
	compositionMode = newCompositionMode;
}

Plotmode Renderer::getPlotMode() const
{
	return plotMode;
}

void Renderer::setPlotMode(Plotmode newPlotMode)
{
	plotMode = newPlotMode;
}

