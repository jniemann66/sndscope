/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#include "plotter.h"

#include "delayline.h"
#include "differentiator.h"

//#define TIME_RENDER_FUNC
#ifdef TIME_RENDER_FUNC
#include "movingaverage.h"
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

Plotter::Plotter(QObject *parent)
	: QObject{parent}
{
}

void Plotter::calcScaling()
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

void Plotter::render(const QVector<QVector<float>> &inputBuffers, int64_t framesAvailable, int64_t currentFrame)
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
	const bool drawLines =  ( (!connectSamplesSweepOnly || connectSamples) &&
							  plotMode == Sweep  &&
							  !panicMode &&
							  (sweepParameters.getSamplesPerSweep() > 25)
							  );

	// todo: whenever upsampling changes, reset this with upsampled value
	int64_t expected = expectedFrames * sweepParameters.upsampleFactor;
	int64_t firstFrameToPlot = catchAllFrames ? 0ll : std::max<int64_t>(0ll, framesAvailable - 2 * expected);

	// calculate all the points to draw
	for(int64_t i = firstFrameToPlot; i < framesAvailable; i++) {

		// types converted here : audio data is float, graphics is qreal (aka double)
		double ch0val = static_cast<double>(inputBuffers[0][i]);
		double ch1val = (numInputChannels > 1 ? static_cast<double>(inputBuffers[1][i]) : 0.0);
		// ---

		switch(plotMode) {
		case XY:
		default:
		{
			QPointF pt{(1.0 + ch0val) * cx, (1.0 - ch1val) * cy};
			static QPointF lastPoint = pt;
			if(drawLines) {
				plotBuffer.append(lastPoint);
			}
			plotBuffer.append(pt);
			lastPoint = pt;
		}
			break;
		case MidSide:
		{
			static constexpr double rsqrt2 = 0.707;
			QPointF pt{(1.0 + rsqrt2 * (ch0val - ch1val)) * cx,
						(1.0 - rsqrt2 * (ch0val + ch1val)) * cy};
			static QPointF lastPoint = pt;
			if(drawLines) {
				plotBuffer.append(lastPoint);
			}
			plotBuffer.append(pt);
			lastPoint = pt;
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

	constexpr bool debugPlotBufferSize = false;
	if constexpr(debugPlotBufferSize) {
		static decltype(plotBuffer.size()) maxSize = 0ll;
		if(plotBuffer.size() > maxSize) {
			maxSize = plotBuffer.size();
			qDebug() << "new size:" << maxSize;
		}
	}

#ifdef SNDSCOPE_BLEND2D
	BLContext ctx;
	ctx.begin(*blImageWrapper->getBlImage());

	blImageWrapper->getBlImage();
	//todo: draw some stuff

#else


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

	if(drawLines) {
		painter.drawLines(plotBuffer);
	} else {
		painter.drawPoints(plotBuffer);
	}
	plotBuffer.clear();



	if(showTrigger) {
		drawTrigger(&painter);
	}

	painter.endNativePainting();
#endif

	freshRender = true;
	emit renderedFrame(currentFrame);
}

void Plotter::drawTrigger(QPainter* painter)
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

bool Plotter::getShowTrigger() const
{
	return showTrigger;
}

void Plotter::setShowTrigger(bool newShowTrigger)
{
	showTrigger = newShowTrigger;
}

bool Plotter::getconnectSamples() const
{
	return connectSamples;
}

void Plotter::setconnectSamples(bool newconnectSamples)
{
	connectSamples = newconnectSamples;
}

#ifdef SNDSCOPE_BLEND2D
BLImageWrapper *Plotter::getBlImageWrapper() const
{
	return blImageWrapper;
}

void Plotter::setBlImageWrapper(BLImageWrapper *newBlImageWrapper)
{
	blImageWrapper = newBlImageWrapper;
}
#endif

SweepParameters Plotter::getSweepParameters() const
{
	return sweepParameters;
}

void Plotter::setSweepParameters(const SweepParameters &newSweepParameters)
{
	sweepParameters = newSweepParameters;
	calcScaling();
}

double Plotter::getTimeLimit_ms() const
{
	return timeLimit_ms;
}

void Plotter::setTimeLimit_ms(double newTimeLimit_ms)
{
	timeLimit_ms = newTimeLimit_ms;
}

bool Plotter::getFreshRender() const
{
	return freshRender;
}

void Plotter::setFreshRender(bool newFreshRender)
{
	freshRender = newFreshRender;
}

QPixmap *Plotter::getPixmap() const
{
	return pixmap;
}

void Plotter::setPixmap(QPixmap *newPixmap)
{
	pixmap = newPixmap;
}

int Plotter::getAudioFramesPerMs() const
{
	return audioFramesPerMs;
}

void Plotter::setAudioFramesPerMs(int newAudioFramesPerMs)
{
	audioFramesPerMs = newAudioFramesPerMs;
}

QColor Plotter::getDarkencolor() const
{
	return darkencolor;
}

void Plotter::setDarkencolor(const QColor &newDarkencolor)
{
	darkencolor = newDarkencolor;
}

int Plotter::getDarkenNthFrame() const
{
	return darkenNthFrame;
}

void Plotter::setDarkenNthFrame(int newDarkenNthFrame)
{
	darkenNthFrame = newDarkenNthFrame;
	darkenCooldownCounter = darkenNthFrame;
}

int64_t Plotter::getExpectedFrames() const
{
	return expectedFrames;
}

void Plotter::setExpectedFrames(int64_t newExpectedFrames)
{
	expectedFrames = newExpectedFrames;
}

qreal Plotter::getBeamWidth() const
{
	return beamWidth;
}

void Plotter::setBeamWidth(qreal newBeamWidth)
{
	beamWidth = newBeamWidth;
}

qreal Plotter::getBeamIntensity() const
{
	return beamIntensity;
}

void Plotter::setBeamIntensity(qreal newBeamIntensity)
{
	beamIntensity = newBeamIntensity;
}

int Plotter::getNumInputChannels() const
{
	return numInputChannels;
}

void Plotter::setNumInputChannels(int newNumInputChannels)
{
	numInputChannels = newNumInputChannels;
}

QColor Plotter::getPhosphorColor() const
{
	return phosphorColor;
}

void Plotter::setPhosphorColor(const QColor &newPhosphorColor)
{
	phosphorColor = newPhosphorColor;
}

QPainter::CompositionMode Plotter::getCompositionMode() const
{
	return compositionMode;
}

void Plotter::setCompositionMode(QPainter::CompositionMode newCompositionMode)
{
	compositionMode = newCompositionMode;
}

Plotmode Plotter::getPlotMode() const
{
	return plotMode;
}

void Plotter::setPlotMode(Plotmode newPlotMode)
{
	plotMode = newPlotMode;
}

