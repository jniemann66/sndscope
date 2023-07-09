/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#include "scopewidget.h"
#include "delayline.h"
#include "differentiator.h"
#include "movingaverage.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QDebug>

#include <cmath>

#define SHOW_AVG_RENDER_TIME
#ifdef SHOW_AVG_RENDER_TIME
#include "functimer.h"
#endif

ScopeWidget::ScopeWidget(QWidget *parent) : QWidget(parent)
{
    scopeDisplay = new ScopeDisplay(this);
	audioController = new AudioController(this);
	auto mainLayout = new QVBoxLayout;
	screenLayout = new QHBoxLayout;

    constexpr int virtualFPS = 100; // number of virtual frames per second
	constexpr int screenFPS = 150;
	constexpr double plotInterval = 1000.0 / virtualFPS; // interval (in ms) between virtual frames
    constexpr double screenUpdateInterval = 1000.0 / screenFPS;

    plotTimer.setInterval(plotInterval);
    screenUpdateTimer.setInterval(screenUpdateInterval);
    scopeDisplay->getPixmap()->fill(backgroundColor);

	const auto divs = scopeDisplay->getDivisions();
	sweepParameters.horizontalDivisions = divs.first;
	sweepParameters.verticalDivisions = divs.second;
	sweepParameters.sweepUnused = {plotMode != Sweep};

	//upsampler.setCoefficients(Interpolator<float, float, upsampleFactor>::minPhase4_coefficients);

	setUpsampling(getUpsampling());

	calcScaling();
    connect(scopeDisplay, &ScopeDisplay::pixmapResolutionChanged, this, [this](){
		calcScaling();
	});

	connect(&plotTimer, &QTimer::timeout, this, [this]{
		if(!paused) {

		//plotTest();
			readInput();

			// send audio to output
			pushOut->write(reinterpret_cast<char*>(rawinputBuffer.data()), framesRead * audioFormat.bytesPerFrame());

			// plot it
			render();
		}
	});

    connect(&screenUpdateTimer, &QTimer::timeout, this, [this] {
        if(!paused) {
            if(freshRender) {
                scopeDisplay->update();
                freshRender = false;
            }
        }
    });

	connect(audioController, &AudioController::outputVolume, this, [this](qreal linearVol){
		emit outputVolume(linearVol);
	});



    screenLayout->addWidget(scopeDisplay, 0, Qt::AlignHCenter);
	mainLayout->addLayout(screenLayout);
	setLayout(mainLayout);

	plotTimer.start();
    screenUpdateTimer.start();
	outputDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
}

QPair<bool, QString> ScopeWidget::loadSoundFile(const QString& filename)
{
	sndfile.reset(new SndfileHandle(filename.toLatin1(), SFM_READ));
	fileLoaded = (sndfile->error() == SF_ERR_NO_ERROR);
	if(fileLoaded) {

		// set up rendering parameters, based on soundfile properties
		numInputChannels = sndfile->channels();

		// initialize raw (interleaved) input buffer
		rawinputBuffer.resize(numInputChannels * sndfile->samplerate()); // 1s of storage

		// initialize channel input buffers
		inputBuffers.resize(numInputChannels);
		for(int ch = 0; ch < numInputChannels; ch++) {
			size_t chbufSize = sndfile->samplerate() * upsampleFactor; // allow for upsampling
			inputBuffers[ch].resize(chbufSize);
		}

		audioFramesPerMs = sndfile->samplerate() / 1000;
		expectedFrames = plotTimer.interval() * audioFramesPerMs;
		msPerAudioFrame = 1000.0 / sndfile->samplerate();
		calcScaling();

		maxFramesToRead = rawinputBuffer.size() / sndfile->channels();

		totalFrames = sndfile->frames();
		returnToStart();

		// set up audio
		audioFormat.setSampleRate(sndfile->samplerate());
		audioFormat.setChannelCount(sndfile->channels());
		audioFormat.setSampleSize(32);
		audioFormat.setCodec("audio/pcm");
		audioFormat.setByteOrder(QAudioFormat::LittleEndian);
		audioFormat.setSampleType(QAudioFormat::Float);
		audioController->initializeAudio(audioFormat, outputDeviceInfo);

		emit loadedFile();
	}

	return {fileLoaded, sndfile->strError()};
}

int ScopeWidget::getLengthMilliseconds() const
{
	return static_cast<int>(msPerAudioFrame * sndfile->frames());
}

bool ScopeWidget::getPaused() const
{
	return paused;
}

void ScopeWidget::setPaused(bool value)
{
	// disallow unpause if file not loaded
	if(!value && paused && !fileLoaded) {
		return;
	}

	paused = value;
	if(!paused) {
		pushOut=audioController->start();
		startFrame = currentFrame;
		elapsedTimer.restart();
	}
}

void ScopeWidget::returnToStart()
{
	elapsedTimer.restart();
	currentFrame = 0ll;
	startFrame = 0ll;

	if(sndfile != nullptr && !sndfile->error()) {
		sndfile->seek(0ll, SEEK_SET);
	}
}

void ScopeWidget::gotoPosition(int64_t milliSeconds)
{
	elapsedTimer.restart();
	if(sndfile != nullptr && !sndfile->error()) {
		currentFrame = audioFramesPerMs * milliSeconds;
		startFrame = currentFrame;
		sndfile->seek(qMin(currentFrame, sndfile->frames()), SEEK_SET);
	}
}

QColor ScopeWidget::getBackgroundColor() const
{
	return backgroundColor;
}

void ScopeWidget::setBackgroundColor(const QColor &value)
{
	backgroundColor = value;
}

void ScopeWidget::setPhosporColors(const QVector<QColor>& colors)
{
	if(!colors.isEmpty()) {
		phosphorColor = colors.at(0);
		if(colors.count() > 1) {
			compositionMode = QPainter::CompositionMode_HardLight;
			darkencolor = colors.at(1);
		} else {
			compositionMode = QPainter::CompositionMode_SourceOver;
			darkencolor = backgroundColor;
		}
	}
}

double ScopeWidget::getPersistence() const
{
	return persistence;
}

void ScopeWidget::setPersistence(double time_ms)
{
	// qDebug() << QStringLiteral("setting persistence to %1").arg(time_ms);
	persistence = time_ms;

	// define fraction of original brightness
	constexpr double decayTarget = 0.2;

	// set minimum darkening amount threshold. (If the darkening amount is too low, traces will never completely disappear)
	constexpr int minDarkenAlpha = 32;

	darkenAlpha = 0;
	darkenNthFrame = 0;
	do {
		++darkenNthFrame; // for really long persistence, darkening operation may need to occur less often than once per frame
		double n = std::max(1.0, time_ms / plotTimer.interval()) / darkenNthFrame; // number of frames to reach decayTarget (can't be zero)
		darkenAlpha = std::min(std::max(1, static_cast<int>(255 * (1.0 - std::pow(decayTarget, (1.0 / n))))), 255);
	} while (darkenAlpha < minDarkenAlpha);

	darkencolor.setAlpha(darkenAlpha);
	darkenCooldownCounter = darkenNthFrame;
}

QColor ScopeWidget::getPhosphorColor() const
{
	return phosphorColor;
}

double ScopeWidget::getFocus() const
{
	return focus;
}

void ScopeWidget::setFocus(double value)
{
	constexpr double maxBeamWidth = 12;
	focus = value;
	beamWidth = qMax(0.5, (1.0 - (focus * 0.01)) * maxBeamWidth);
	beamIntensity = 8.0 / (beamWidth * beamWidth);
	calcBeamAlpha();
}

double ScopeWidget::getBrightness() const
{
	return brightness;
}

void ScopeWidget::setBrightness(double value)
{
	brightness = value;
	calcBeamAlpha();
}

void ScopeWidget::calcBeamAlpha()
{
	beamAlpha = qMin(1.27 * brightness * beamIntensity, 255.0);
	phosphorColor.setAlpha(beamAlpha);
}

int64_t ScopeWidget::getTotalFrames() const
{
	return totalFrames;
}

void ScopeWidget::setTotalFrames(const int64_t &value)
{
	totalFrames = value;
}

void ScopeWidget::readInput()
{
	// estimate how far ahead to read
	const int64_t toFrame = qMin(totalFrames - 1, startFrame + static_cast<int64_t>(elapsedTimer.elapsed() * audioFramesPerMs));

	// read from file
	framesRead = sndfile->readf(rawinputBuffer.data(), qMin(maxFramesToRead, toFrame - currentFrame));
	currentFrame += framesRead;

	constexpr bool debugExpectedFrames = false;
	if constexpr(debugExpectedFrames) {
		if(framesRead > expectedFrames)
			qDebug() << "expected" << expectedFrames << "got" << framesRead;
	}

	// de-interleave
	if(upsampling && numInputChannels <= 2) {
		if(numInputChannels == 1) {
			upsampler.upsampleBlockMono(inputBuffers[0].data(), rawinputBuffer.constData(), framesRead);
			framesAvailable = framesRead * upsampleFactor;
		} else if(numInputChannels == 2) {
			upsampler.upsampleBlockStereo(inputBuffers[0].data(), inputBuffers[1].data(), rawinputBuffer.constData(), framesRead);
			framesAvailable = framesRead * upsampleFactor;
		}
	} else {
		for(int64_t f = 0ll; f < framesRead; f++) {
			for(int ch = 0; ch < numInputChannels; ch++) {
				inputBuffers[ch][f] = rawinputBuffer[f * numInputChannels + ch];
			}
		}
		framesAvailable = framesRead;
	}
}

void ScopeWidget::render()
{
    constexpr size_t historyLength = 10;
    static MovingAverage<double, historyLength> movingAverage;
	static int64_t callCount = 0;
	static double renderTime = 0.0;
	static double total_renderTime = 0.0;

	// collect data for overall average
	total_renderTime += renderTime;
	callCount++;

    const double mov_avg_renderTime = movingAverage.get(renderTime);
	const bool panicMode {mov_avg_renderTime > plotTimer.interval()};

#ifdef SHOW_AVG_RENDER_TIME
	if(panicMode) {
		qDebug() << QStringLiteral("Panic: recent avg (%1ms) > plot-interval (%2ms)")
					.arg(mov_avg_renderTime, 0, 'f', 2)
					.arg(plotTimer.interval());
	}

	if(callCount % 100 == 0) {
		qDebug() << QStringLiteral("Avg render time: Overall=%1ms Recent(last %2)=%3ms")
					.arg(total_renderTime / std::max(1ll, callCount), 0, 'f', 2)
					.arg(historyLength)
					.arg(mov_avg_renderTime, 0, 'f', 2);
	}
#endif

	FuncTimer<std::chrono::milliseconds> funcTimer(&renderTime);

	constexpr bool catchAllFrames = false;
	constexpr double rsqrt2 = 0.707;
	const bool drawLines =  ( sweepParameters.connectDots &&
							  plotMode == Sweep  &&
							  !panicMode &&
							  (sweepParameters.getSamplesPerSweep() > 25)
							  );


    auto pixmap = scopeDisplay->getPixmap();
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

	int64_t expected = expectedFrames * (upsampling ? upsampleFactor : 1);
	int64_t firstFrameToPlot = catchAllFrames ? 0ll : std::max(0ll, framesAvailable - 2 * expected);

	// draw
	const double w = 2.0 * cx;
	for(int64_t i = firstFrameToPlot; i < framesAvailable; i++) {

		double ch0val = inputBuffers[0][i];
		double ch1val = (numInputChannels > 1 ? inputBuffers[1][i] : 0.0);

		switch(plotMode) {
		case XY:
		default:
			plotBuffer.append({(1.0 + ch0val) * cx, (1.0 - ch1val) * cy});
			break;
		case MidSide:
            plotBuffer.append({(1.0 + rsqrt2 * (ch0val - ch1val)) * cx,
                               (1.0 - rsqrt2 * (ch0val + ch1val)) * cy});
			break;
		case Sweep:
		{
			static Differentiator<double> d;
			static DelayLine<double, d.delayTime> delayLine;
			static bool triggered = false;
			static double x = 0.0;
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
		renderTrigger(&painter);
	}

	painter.endNativePainting();
	freshRender = true;
	emit renderedFrame(currentFrame * msPerAudioFrame);
}

void ScopeWidget::plotTest()
{
	constexpr size_t historyLength = 10;
	static MovingAverage<double, historyLength> movingAverage;
	static int64_t callCount = 0;
	static double renderTime = 0.0;

	FuncTimer<std::chrono::milliseconds> funcTimer(&renderTime);

	// collect data for overall average
	callCount++;

	const double mov_avg_renderTime = movingAverage.get(renderTime);

	const double mov_avg_time_per_point = 1000.0 * mov_avg_renderTime / testPlot.size();

	if(callCount % 100 == 0) {
		qDebug() << QStringLiteral("Mov Avg=%1ms (%2 μS per plot point)")
					.arg(mov_avg_renderTime, 0, 'f', 2)
					.arg(mov_avg_time_per_point, 0, 'f', 2);
	}

	auto pixmap = scopeDisplay->getPixmap();
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
	painter.setRenderHint(QPainter::Antialiasing, true);
	const QPen pen{phosphorColor, beamWidth, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin};
	painter.setPen(pen);

	painter.drawPoints(testPlot.data(), testPlot.size());
	painter.endNativePainting();
}

void ScopeWidget::makeTestPlot()
{
	testPlot.clear();

	const double d = (2 * M_PI) / w;
	for(int x = 0; x < w; x++) {
		testPlot.append(QPointF{static_cast<qreal>(x) , cy - cy* sin(x * d)});
	}
}


void ScopeWidget::renderTrigger(QPainter* painter)
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

void ScopeWidget::wipeScreen()
{
    auto pixmap = scopeDisplay->getPixmap();
    QPainter painter(pixmap);
	painter.setCompositionMode(compositionMode);

	QColor d{darkencolor};
	d.setAlpha(255);

    // darken:
    painter.fillRect(pixmap->rect(), d);
	scopeDisplay->update();
}

QAudioDeviceInfo ScopeWidget::getOutputDeviceInfo() const
{
	return outputDeviceInfo;
}

void ScopeWidget::setOutputDevice(const QAudioDeviceInfo &newOutputDeviceInfo)
{
	bool changed = (outputDeviceInfo != newOutputDeviceInfo);
	outputDeviceInfo = newOutputDeviceInfo;
	if(changed && fileLoaded) {
		audioController->initializeAudio(audioFormat, outputDeviceInfo);
	}
}

Plotmode ScopeWidget::getPlotmode() const
{
	return plotMode;
}

void ScopeWidget::setPlotmode(Plotmode newPlotmode)
{
	plotMode = newPlotmode;
	if(plotMode == Sweep) {
		//
	} else {
		sweepParameters.sweepUnused = true;
	}
}

bool ScopeWidget::getUpsampling() const
{
	return upsampling;
}

void ScopeWidget::setUpsampling(bool val)
{
	upsampling = val;
	sweepParameters.setUpsampleFactor(upsampling ? static_cast<double>(upsampleFactor) : 1.0);
	if(upsampling) {
		upsampler.reset();
	}
}

void ScopeWidget::calcScaling()
{
    auto pixmap = scopeDisplay->getPixmap();
	w = pixmap->width();
	h = pixmap->height();
	cx = w / 2;
    cy = pixmap->height() / 2;
	const auto a = scopeDisplay->getAspectRatio();
	divx = w / a.first;
	divy = h / a.second;

	plotBuffer.reserve(4 * plotTimer.interval() * audioFramesPerMs);
	// makeTestPlot();
	sweepParameters.setWidthFrameRate(w, audioFramesPerMs);
}



void ScopeWidget::setAudioVolume(qreal linearVolume)
{
	audioController->setOutputVolume(linearVolume);
}

void ScopeWidget::setSweepParameters(const SweepParameters &newSweepParameters)
{
	double newDuration = newSweepParameters.duration_ms;
	if(sweepParameters.duration_ms != newDuration) {
		sweepParameters.setDuration_ms(newDuration);
	}

	const double &newTriggerLevel = newSweepParameters.triggerLevel;
	const double &newTriggerTolerance = newSweepParameters.triggerTolerance;
	if(sweepParameters.triggerLevel != newTriggerLevel || sweepParameters.triggerTolerance != newTriggerTolerance) {
		sweepParameters.triggerLevel = newTriggerLevel;
		sweepParameters.triggerTolerance = newTriggerTolerance;
		sweepParameters.triggerMin  = newTriggerLevel - sweepParameters.triggerTolerance;
		sweepParameters.triggerMax  = newTriggerLevel + sweepParameters.triggerTolerance;
		if(paused) {
			setShowTrigger(showTrigger);
		}
	}

	sweepParameters.slope = newSweepParameters.slope;
	sweepParameters.triggerEnabled = newSweepParameters.triggerEnabled;
	sweepParameters.connectDots = newSweepParameters.connectDots;
}

bool ScopeWidget::getShowTrigger() const
{
	return showTrigger;
}

void ScopeWidget::setShowTrigger(bool val)
{
	showTrigger = (plotMode == Sweep) && val;
	if(paused) {

		auto pixmap = scopeDisplay->getPixmap();
		QPainter painter(pixmap);
		painter.beginNativePainting();
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.fillRect(pixmap->rect(), Qt::black);

		if(showTrigger) {
			renderTrigger(&painter);
		}
		painter.endNativePainting();
		scopeDisplay->update();
	}
}

SweepParameters ScopeWidget::getSweepParameters() const
{
	return sweepParameters;
}













