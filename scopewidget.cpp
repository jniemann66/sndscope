/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#include "scopewidget.h"

#include <QVBoxLayout>
#include <QEvent>
#include <QDebug>

#include <cmath>

ScopeWidget::ScopeWidget(QWidget *parent) : QWidget(parent)
{
    scopeDisplay = new ScopeDisplay(this);
	audioController = new AudioController(this);
	renderer = new Renderer(this);

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

	setUpsampling(getUpsampling());

	renderer->setTimeLimit_ms(plotInterval);
	renderer->setPixmap(scopeDisplay->getPixmap());
	renderer->setSweepParameters(sweepParameters);

    connect(scopeDisplay, &ScopeDisplay::pixmapResolutionChanged, this, [this](){
		const auto p = scopeDisplay->getPixmap();
		renderer->setPixmap(p);
		w = p->width();
		h = p->height();
		sweepParameters.setWidthFrameRate(w, audioFramesPerMs);
		renderer->calcScaling();
	});

	connect(&plotTimer, &QTimer::timeout, this, [this]{
		if(!paused) {

			readInput();

			// send audio to output
			pushOut->write(reinterpret_cast<char*>(rawinputBuffer.data()), framesRead * audioFormat.bytesPerFrame());

			// plot it
			renderer->render(inputBuffers, framesAvailable, currentFrame);
		}
	});

    connect(&screenUpdateTimer, &QTimer::timeout, this, [this] {
        if(!paused) {
			if(renderer->getFreshRender()) {
				scopeDisplay->update();
				renderer->setFreshRender(false);
            }
        }
    });

	connect(audioController, &AudioController::outputVolume, this, [this](qreal linearVol){
		emit outputVolume(linearVol);
	});

	connect(renderer, &Renderer::renderedFrame, this, [this](int64_t frame){
		emit renderedFrame(frame * msPerAudioFrame);
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
		msPerAudioFrame = 1000.0 / sndfile->samplerate();
		expectedFrames = plotTimer.interval() * audioFramesPerMs;
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

		renderer->setExpectedFrames(expectedFrames);
		renderer->setAudioFramesPerMs(audioFramesPerMs);
		renderer->setNumInputChannels(audioFormat.channelCount());
		renderer->calcScaling();

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
		renderer->setPhosphorColor(colors.at(0));
		if(colors.count() > 1) {
			renderer->setCompositionMode(QPainter::CompositionMode_HardLight);
			renderer->setDarkencolor(colors.at(1));
		} else {
			renderer->setCompositionMode(QPainter::CompositionMode_SourceOver);
			renderer->setDarkencolor(backgroundColor);
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

	int darkenAlpha = 0;
	int  darkenNthFrame = 0;
	do {
		++darkenNthFrame; // for really long persistence, darkening operation may need to occur less often than once per frame
		double n = std::max(1.0, time_ms / plotTimer.interval()) / darkenNthFrame; // number of frames to reach decayTarget (can't be zero)
		darkenAlpha = std::min(std::max(1, static_cast<int>(255 * (1.0 - std::pow(decayTarget, (1.0 / n))))), 255);
	} while (darkenAlpha < minDarkenAlpha);

	auto darkenColor = renderer->getDarkencolor();
	darkenColor.setAlpha(darkenAlpha);
	renderer->setDarkencolor(darkenColor);
	renderer->setDarkenNthFrame(darkenNthFrame);
}

QColor ScopeWidget::getPhosphorColor() const
{
	return renderer->getPhosphorColor();
}

double ScopeWidget::getFocus() const
{
	return focus;
}

void ScopeWidget::setFocus(double value)
{
	constexpr double maxBeamWidth = 12;
	focus = value;
	renderer->setBeamWidth(qMax(0.5, (1.0 - (focus * 0.01)) * maxBeamWidth));
	const auto beamWidth = renderer->getBeamWidth();
	renderer->setBeamIntensity(8.0 / (beamWidth * beamWidth));
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
	beamAlpha = qMin(1.27 * brightness * renderer->getBeamIntensity(), 255.0);
	auto phosphorColor = renderer->getPhosphorColor();
	phosphorColor.setAlpha(beamAlpha);
	renderer->setPhosphorColor(phosphorColor);
}

int64_t ScopeWidget::getTotalFrames() const
{
	return totalFrames;
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

void ScopeWidget::wipeScreen()
{
    auto pixmap = scopeDisplay->getPixmap();
    QPainter painter(pixmap);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	QColor d{backgroundColor};
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
	renderer->setPlotMode(plotMode);
}

bool ScopeWidget::getUpsampling() const
{
	return upsampling;
}

void ScopeWidget::setUpsampling(bool val)
{
	upsampling = val;
	sweepParameters.setUpsampleFactor(upsampling ? static_cast<double>(upsampleFactor) : 1.0);
	renderer->setSweepParameters(sweepParameters);
	if(upsampling) {
		upsampler.reset();
	}
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
	sweepParameters.setWidthFrameRate(w, audioFramesPerMs);
	if(renderer != nullptr) {
		renderer->setSweepParameters(sweepParameters);
	}
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
			renderer->drawTrigger(&painter);
		}
		painter.endNativePainting();
		scopeDisplay->update();
	} else {
		renderer->setShowTrigger(showTrigger);
	}
}


SweepParameters ScopeWidget::getSweepParameters() const
{
	return sweepParameters;
}




