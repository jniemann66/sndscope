#include "scopewidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QResizeEvent>
#include <QDebug>

ScopeWidget::ScopeWidget(QWidget *parent) : QWidget(parent), pixmap(480, 480)
{
    auto mainLayout = new QVBoxLayout;
    screenWidget = new QLabel;
    sizeTracker = new SizeTracker(this);

    screenWidget->setPixmap(pixmap);
    pixmap.fill(Qt::black);
    screenWidget->setScaledContents(true);
    darkenTimer.setInterval(1000.0 / 50);

    screenWidget->installEventFilter(sizeTracker);

    connect(&darkenTimer, &QTimer::timeout, this, [this]{
       plot();
       screenWidget->setPixmap(pixmap);
       emit renderedFrame(frame * millisecondsPerSample);
    });

    darkenTimer.start();
    mainLayout->addWidget(screenWidget);
    setLayout(mainLayout);

}

QPair<bool, QString> ScopeWidget::loadSoundFile(const QString& filename)
{
    h.reset(new SndfileHandle(filename.toLatin1(), SFM_READ));
    if(h->error() == SF_ERR_NO_ERROR) {
        inputBuffer.resize(h->channels() * h->samplerate()); // 1s of storage
        samplesPerMillisecond = h->samplerate() / 1000;
        millisecondsPerSample = 1000.0 / h->samplerate();
    }

    elapsedTimer.start();
    return {h->error() == SF_ERR_NO_ERROR, h->strError()};
}

int ScopeWidget::getLengthMilliseconds()
{
    return static_cast<int>(millisecondsPerSample * h->frames());
}

int ScopeWidget::heightForWidth(int) const
{
    return width();
}

void ScopeWidget::plot()
{
    int64_t toFrame = qMin(h->frames() - 1, static_cast<int64_t>(elapsedTimer.elapsed() * samplesPerMillisecond));
    int64_t framesRead = h->readf(inputBuffer.data(), toFrame - frame);
    frame += framesRead;

    QPainter painter(&pixmap);

    // darken:
    painter.fillRect(screenWidget->pixmap().rect(), {QColor{0, 0, 0, 250}});

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
