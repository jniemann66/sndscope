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

ScopeWidget::ScopeWidget(QWidget *parent) : QWidget(parent), pixmap(640, 640)
{
    auto mainLayout = new QVBoxLayout;
    auto screenLayout = new QHBoxLayout;
    screenWidget = new QLabel;
    sizeTracker = new SizeTracker(this);

    screenWidget->setPixmap(pixmap);
    pixmap.fill(Qt::black);
    screenWidget->setScaledContents(true);
    darkenTimer.setInterval(1000.0 / 200);
    setBrightness(32.0);
    setFocus(50.0);

    screenWidget->installEventFilter(sizeTracker);

    connect(&darkenTimer, &QTimer::timeout, this, [this]{
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

    darkenTimer.start();
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

double ScopeWidget::getFocus() const
{
    return focus;
}

void ScopeWidget::setFocus(double value)
{
    focus = value;
    beamWidth = value * 0.01 * 8;
}

double ScopeWidget::getBrightness() const
{
    return brightness;
}

void ScopeWidget::setBrightness(double value)
{
    brightness = value;
    beamAlpha = 0.63 * value;
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

    // darken:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    // since 5.15, use the return-by-value version of pixmap()
    painter.fillRect(screenWidget->pixmap().rect(), {QColor{0, 0, 0, 32}});
#else
    // prior to 5.15, use the return-by-pointer version or pixmap()
    painter.fillRect(screenWidget->pixmap()->rect(), {QColor{0, 0, 0, 32}});
#endif

    // prepare pen
    QPen pen{QColor{94, 255, 0, beamAlpha}, beamWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};

    // set pen
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);

    // draw
    for(int64_t i = 0; i < framesRead; i++ ) {
        int64_t j = 2 * i;
        double x = (1.0 + inputBuffer.at(j)) * sizeTracker->cx;
        double y = (1.0 - inputBuffer.at(j + 1)) * sizeTracker->cx;
        painter.drawPoint(x,y);
    }
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
