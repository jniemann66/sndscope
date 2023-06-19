/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "scopewidget.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <cmath>

ScopeWidget::ScopeWidget(QWidget *parent) : QWidget(parent), pixmap(640, 640)
{
    screenWidget = new PictureBox(&pixmap);
    audioController = new AudioController(this);
    auto mainLayout = new QVBoxLayout;
    screenLayout = new QHBoxLayout;

    constexpr int virtualFPS = 100; // number of virtual frames per second
    constexpr int screenFPS = 50;
    constexpr int v_to_s_Ratio = virtualFPS / screenFPS; // number of virtual frames per screen frame
    constexpr double plotInterval = 1000.0 / virtualFPS; // interval (in ms) between virtual frames

    setConstrainToSquare(constrainToSquare);
    pixmap.fill(Qt::black);
    plotTimer.setInterval(plotInterval);
    setBrightness(66.0);
    setFocus(50.0);
    setPersistence(32);
    calcCenter();

    connect(screenWidget, &PictureBox::pixmapResolutionChanged, this, [this](){
        calcCenter();
    });

    connect(&plotTimer, &QTimer::timeout, this, [this]{
        if(!paused) {
            render();
            screenDrawCounter++;
            if(screenDrawCounter == v_to_s_Ratio) {
                screenWidget->setPixmap(pixmap);
                screenDrawCounter = 0;
            }
            emit renderedFrame(currentFrame * millisecondsPerFrame);
        }
    });

    plotTimer.start();

    // !!

    mainLayout->addLayout(screenLayout);
    setLayout(mainLayout);
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
        auto device = QAudioDeviceInfo::defaultOutputDevice();
        audioController->initializeAudio(audioFormat, device);
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

bool ScopeWidget::getMultiColor() const
{
    return multiColor;
}

void ScopeWidget::setMultiColor(bool value, const QColor& altColor)
{
    multiColor = value;
    if(multiColor) {
        compositionMode = QPainter::CompositionMode_HardLight;
        darkencolor = altColor;
    } else {
        compositionMode = QPainter::CompositionMode_SourceOver;
        darkencolor = backgroundColor;
    }
}

double ScopeWidget::getPersistence() const
{
    return persistence;
}

void ScopeWidget::setPersistence(double value)
{
    persistence = value;
    // define fraction of original brightness (10%)
    constexpr double decayTarget = 0.1;
    // set minimum darkening amount threshold. (If the darkening amount is too low, traces will never completely disappear)
    constexpr int minDarkenAlpha = 32;
    darkenNthFrame = 0;
    do {
        ++darkenNthFrame;
        double n = std::max(0.01, value / plotTimer.interval()) / darkenNthFrame; // number of frames to reach decayTarget (can't be zero)
        darkenAlpha = std::min(std::max(1, static_cast<int>(255 * (1.0 - std::pow(decayTarget, (1.0 / n))))), 255);

    } while (darkenAlpha < minDarkenAlpha);
    darkenCooldownCounter = darkenNthFrame;
}

QRgb ScopeWidget::getPhosphorColor() const
{
    return phosphorColor;
}

void ScopeWidget::setPhosphorColor(const QRgb &value)
{
    phosphorColor = value;
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
    const int64_t toFrame = qMin(totalFrames - 1, startFrame + static_cast<int64_t>(elapsedTimer.elapsed() * framesPerMillisecond));
    const int64_t framesRead = sndfile->readf(inputBuffer.data(), qMin(maxFramesToRead, toFrame - currentFrame));

    QPainter painter(&pixmap);
    painter.setCompositionMode(compositionMode);

    if(--darkenCooldownCounter == 0) {
        // darken:
        QColor d{darkencolor};
        d.setAlpha(darkenAlpha);
        painter.fillRect(pixmap.rect(), d);
        darkenCooldownCounter = darkenNthFrame;
   }

    // prepare pen
    QColor c{phosphorColor};
    c.setAlpha(beamAlpha);
    const QPen pen{c, beamWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};

    // set pen
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);

    // draw
    for(int64_t i = 0; i < 2 * framesRead; i+= 2 ) {
        // get sample values for each channel
        float ch0val = inputBuffer.at(i);
        float ch1val = inputBuffer.at(i + 1);
        // plot point
        float x = (1.0 + ch0val) * cx;
        float y = (1.0 - ch1val) * cy;
        painter.drawPoint(QPointF{x,y});
    }

    const int bytesPerFrame = audioFormat.bytesPerFrame();
    pushOut->write(reinterpret_cast<char*>(inputBuffer.data()), framesRead * bytesPerFrame);
    currentFrame += framesRead;
}

void ScopeWidget::wipeScreen()
{
    QPainter painter(&pixmap);
    painter.setCompositionMode(compositionMode);

    QColor d{darkencolor};
    d.setAlpha(255);

    // darken:
    painter.fillRect(pixmap.rect(), d);

    screenDrawCounter = 0;
}

bool ScopeWidget::getConstrainToSquare() const
{
    return constrainToSquare;
}

void ScopeWidget::setConstrainToSquare(bool value)
{
    constrainToSquare = value;
    screenWidget->setConstrainToSquare(value);

    for(int i = 0; i < screenLayout->count(); i++) {
        screenLayout->removeItem(screenLayout->itemAt(i));
    }

    if(constrainToSquare) {
        screenLayout->addStretch();
    }

    screenLayout->addWidget(screenWidget);

    if(constrainToSquare) {
        screenLayout->addStretch();
    }
}

void ScopeWidget::calcCenter()
{
    cx = pixmap.width() / 2;
    cy = pixmap.height() / 2;
}

void PictureBox::setAllowPixmapResolutionChange(bool value)
{
    allowPixmapResolutionChange = value;
}

bool PictureBox::getAllowPixmapResolutionChange() const
{
    return allowPixmapResolutionChange;
}

bool PictureBox::getConstrainToSquare() const
{
    return constrainToSquare;
}

void PictureBox::setConstrainToSquare(bool value)
{
    constrainToSquare = value;
}
