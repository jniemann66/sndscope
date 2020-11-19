/*
* Copyright (C) 2020 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope
*/

#include "scopewidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QResizeEvent>
#include <QDebug>
#include <cmath>

ScopeWidget::ScopeWidget(QWidget *parent) : QWidget(parent), pixmap(640, 640)
{
    auto mainLayout = new QVBoxLayout;
    auto screenLayout = new QHBoxLayout;
    screenWidget = new QLabel;
    sizeTracker = new SizeTracker(this);

    screenWidget->setPixmap(pixmap);
    pixmap.fill(Qt::black);
    screenWidget->setScaledContents(true);
    plotTimer.setInterval(1000.0 / 200);
    setBrightness(66.0);
    setFocus(50.0);
    setPersistence(32);

    screenWidget->installEventFilter(sizeTracker);

    connect(&plotTimer, &QTimer::timeout, this, [this]{
        if(!paused) {
            render();
            screenDrawCounter++;
            if(screenDrawCounter == 4) {
                screenWidget->setPixmap(pixmap);
                screenDrawCounter = 0;
            }
            emit renderedFrame(currentFrame * millisecondsPerSample);
        }
    });

    plotTimer.start();
    screenLayout->addStretch();
    screenLayout->addWidget(screenWidget);
    screenLayout->addStretch();
    mainLayout->addStretch();
    mainLayout->addLayout(screenLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);
}

QPair<bool, QString> ScopeWidget::loadSoundFile(const QString& filename)
{
    h.reset(new SndfileHandle(filename.toLatin1(), SFM_READ));
    fileLoaded = (h->error() == SF_ERR_NO_ERROR);
    if(fileLoaded) {

        // set up audio output, based on soundfile properties
        audioFormat.setSampleRate(h->samplerate());
        audioFormat.setChannelCount(h->channels());
        audioFormat.setSampleSize(32);
        audioFormat.setCodec("audio/pcm");
        audioFormat.setByteOrder(QAudioFormat::LittleEndian);
        audioFormat.setSampleType(QAudioFormat::Float);
        auto audioDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
        audioOutputQueue.setConfiguration(audioDeviceInfo, audioFormat);
        qDebug() << audioDeviceInfo.deviceName();
      //  audioOutputQueue.play();

        // set up rendering parameters, based on soundfile properties
        inputBuffer.resize(h->channels() * h->samplerate()); // 1s of storage
        samplesPerMillisecond = h->samplerate() / 1000;
        millisecondsPerSample = 1000.0 / h->samplerate();
        maxFramesToRead = inputBuffer.size() / h->channels();
        totalFrames = h->frames();
        returnToStart();
    }

    return {fileLoaded, h->strError()};
}

int ScopeWidget::getLengthMilliseconds()
{
    return static_cast<int>(millisecondsPerSample * h->frames());
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
        startFrame = currentFrame;
        elapsedTimer.restart();
    }
}

void ScopeWidget::returnToStart()
{
    elapsedTimer.restart();
    currentFrame = 0ll;
    startFrame = 0ll;

    if(h != nullptr && !h->error()) {
        h->seek(0ll, SEEK_SET);
    }
}

void ScopeWidget::gotoPosition(int64_t milliSeconds)
{
    elapsedTimer.restart();
    if(h != nullptr && !h->error()) {
        currentFrame = samplesPerMillisecond * milliSeconds;
        startFrame = currentFrame;
        h->seek(qMin(currentFrame, h->frames()), SEEK_SET);
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
    int64_t toFrame = qMin(totalFrames - 1, startFrame + static_cast<int64_t>(elapsedTimer.elapsed() * samplesPerMillisecond));
    int64_t framesRead = h->readf(inputBuffer.data(), qMin(maxFramesToRead, toFrame - currentFrame));
    currentFrame += framesRead;

    QPainter painter(&pixmap);
    painter.setCompositionMode(compositionMode);

    if(--darkenCooldownCounter == 0) {

        QColor d{darkencolor};
        d.setAlpha(darkenAlpha);

        // darken:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        // since 5.15, use the return-by-value version of pixmap()
        painter.fillRect(screenWidget->pixmap().rect(), d);
#else
        // prior to 5.15, use the return-by-pointer version or pixmap()
        painter.fillRect(screenWidget->pixmap()->rect(), d);
#endif
        darkenCooldownCounter = darkenNthFrame;
    }

    // prepare pen
    QColor c{phosphorColor};
    c.setAlpha(beamAlpha);
    QPen pen{c, beamWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};

    // set pen
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);

    // draw
    for(int64_t i = 0; i < 2 * framesRead; i+= 2 ) {
        // get sample values for each channel
        float ch0val = inputBuffer.at(i);
        float ch1val = inputBuffer.at(i + 1);
        // plot point
        float x = (1.0 + ch0val) * sizeTracker->cx;
        float y = (1.0 - ch1val) * sizeTracker->cx;
        painter.drawPoint(QPointF{x,y});
        // send audio
        //audioOutputQueue.addAudio(ch0val);
        //audioOutputQueue.addAudio(ch1val);
    }

    qDebug() << audioOutputQueue.size();
}

SizeTracker::SizeTracker(QObject *parent) : QObject(parent)
{
}

bool SizeTracker::eventFilter(QObject *obj, QEvent* event)
{
    if(event->type() == QEvent::Resize) {
        QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
        auto sz = resizeEvent->size();
        w = sz.width();
        h = sz.height();
        cx = w / 2;
        cy = h / 2;
    }

    return QObject::eventFilter(obj, event);
}
