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
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <differentiator.h>

#include <cmath>

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

	calcCenter();
    connect(scopeDisplay, &ScopeDisplay::pixmapResolutionChanged, this, [this](){
		calcCenter();
	});

	connect(&plotTimer, &QTimer::timeout, this, [this]{
		if(!paused) {
			render();
            freshRender = true;
			emit renderedFrame(currentFrame * millisecondsPerFrame);
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
		inputBuffer.resize(sndfile->channels() * sndfile->samplerate()); // 1s of storage
		framesPerMillisecond = sndfile->samplerate() / 1000;
		millisecondsPerFrame = 1000.0 / sndfile->samplerate();
		maxFramesToRead = inputBuffer.size() / sndfile->channels();
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
	}

	return {fileLoaded, sndfile->strError()};
}

int ScopeWidget::getLengthMilliseconds() const
{
	return static_cast<int>(millisecondsPerFrame * sndfile->frames());
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
		currentFrame = framesPerMillisecond * milliSeconds;
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
	focus = value;
	beamWidth = qMax(0.5, value * 0.01 * 8);
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

void ScopeWidget::render()
{
	static Differentiator<double> d;

	const int64_t toFrame = qMin(totalFrames - 1, startFrame + static_cast<int64_t>(elapsedTimer.elapsed() * framesPerMillisecond));
	const int64_t framesRead = sndfile->readf(inputBuffer.data(), qMin(maxFramesToRead, toFrame - currentFrame));

    auto pixmap = scopeDisplay->getPixmap();
    QPainter painter(pixmap);
	painter.setCompositionMode(compositionMode);

	if(--darkenCooldownCounter == 0) {
        // darken:
        painter.fillRect(pixmap->rect(), darkencolor);
		darkenCooldownCounter = darkenNthFrame;
	}

	// set pen
	painter.setRenderHint(QPainter::Antialiasing);
	const QPen pen{phosphorColor, beamWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
	painter.setPen(pen);

	// draw
	for(int64_t i = 0; i < 2 * framesRead; i+= 2 ) {

		float ch0val = inputBuffer.at(i);
		float ch1val = inputBuffer.at(i + 1);

		QPointF pt;
		switch(channelMode) {
		case XY:
		default:
			pt = {(1.0 + ch0val) * cx, (1.0 - ch1val) * cy};
			painter.drawPoint(pt);
			break;
		case MidSide:
		{
			constexpr double rsqrt2 = 0.707;
			pt = {(1.0 + rsqrt2*(ch0val - ch1val)) * cx,
				  (1.0 - rsqrt2*(ch0val + ch1val)) * cy};
			painter.drawPoint(pt);
			break;
		}
		case Single:
		{
			constexpr double thresh = 0.01;
			static const double sweepAdvance = 0.1 * cx / framesPerMillisecond;
			static bool triggered = false;
			static double x = 0.0;
			static QPointF lastPoint{x, cy};

			double slope = d.get(ch0val);
			triggered = triggered || (std::abs(ch0val) < thresh && slope > 0);

			if(triggered) {
				pt = {x, cy * (1.0 - ch0val)};
				painter.drawLine(lastPoint, pt);
				lastPoint = pt;
				x += sweepAdvance;
				if(x >= 2.0 * cx) { // sweep completed
					triggered = false;
					lastPoint = {x = 0.0, cy};
				}
			}
		}
		}
	}

	const int bytesPerFrame = audioFormat.bytesPerFrame();
	pushOut->write(reinterpret_cast<char*>(inputBuffer.data()), framesRead * bytesPerFrame);
	currentFrame += framesRead;
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

void ScopeWidget::setOutputDeviceInfo(const QAudioDeviceInfo &newOutputDeviceInfo)
{
	outputDeviceInfo = newOutputDeviceInfo;
}

ChannelMode ScopeWidget::getChannelMode() const
{
	return channelMode;
}

void ScopeWidget::setChannelMode(ChannelMode newChannelMode)
{
	channelMode = newChannelMode;
}

bool ScopeWidget::getConstrainToSquare() const
{
	return constrainToSquare;
}

void ScopeWidget::setConstrainToSquare(bool value)
{
	constrainToSquare = value;
    scopeDisplay->setConstrainToSquare(value);
//    scopeDisplay->adjustSize();
//    qDebug() << scopeDisplay->sizePolicy();
}

void ScopeWidget::calcCenter()
{
    auto pixmap = scopeDisplay->getPixmap();
    cx = pixmap->width() / 2;
    cy = pixmap->height() / 2;
}

void ScopeWidget::setAudioVolume(qreal linearVolume)
{
	audioController->setOutputVolume(linearVolume);
}
