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
    darkenTimer.setInterval(1000.0 / 50);

    screenWidget->installEventFilter(sizeTracker);

    connect(&darkenTimer, &QTimer::timeout, this, [this]{
        if(!paused) {
            render();
            screenWidget->setPixmap(pixmap);
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
    if(h->error() == SF_ERR_NO_ERROR) {
        inputBuffer.resize(h->channels() * h->samplerate()); // 1s of storage
        samplesPerMillisecond = h->samplerate() / 1000;
        millisecondsPerSample = 1000.0 / h->samplerate();
        maxFramesToRead = inputBuffer.size() / h->channels();
        returnToStart();
    }

    return {h->error() == SF_ERR_NO_ERROR, h->strError()};
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

void ScopeWidget::render()
{
    int64_t toFrame = qMin(h->frames() - 1, startFrame + static_cast<int64_t>(elapsedTimer.elapsed() * samplesPerMillisecond));
    int64_t framesRead = h->readf(inputBuffer.data(), qMin(maxFramesToRead, toFrame - currentFrame));
    currentFrame += framesRead;

    QPainter painter(&pixmap);

    // darken:
    painter.fillRect(screenWidget->pixmap().rect(), {QColor{10, 10, 10, 128}});

    // prepare pen
    //QPen pen{QColor{255,255,255,20}, 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    QPen pen{QColor{94, 255, 0, 20}, 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    //
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);

    // draw:
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
