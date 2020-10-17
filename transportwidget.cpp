#include "transportwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>

TransportWidget::TransportWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* sliderLayout = new QHBoxLayout;
    QHBoxLayout* buttonLayout = new QHBoxLayout;

    slider = new QSlider(Qt::Horizontal);
    rtsButton = new QPushButton;
    playPauseButton = new QPushButton;
    hh = new QLCDNumber(2);
    mm = new QLCDNumber(2);
    ss = new QLCDNumber(2);
    ms = new QLCDNumber(3);

    hh->setDecMode();
    mm->setDecMode();
    ss->setDecMode();
    ms->setDecMode();
    hh->setSegmentStyle(QLCDNumber::Filled);
    mm->setSegmentStyle(QLCDNumber::Filled);
    ss->setSegmentStyle(QLCDNumber::Filled);
    ms->setSegmentStyle(QLCDNumber::Filled);

    playPauseButton->setIcon(QIcon{":/icons/play-solid.png"});
    rtsButton->setIcon(QIcon{":/icons/step-backward-solid.png"});

    sliderLayout->addWidget(slider);
    buttonLayout->addWidget(rtsButton);
    buttonLayout->addWidget(playPauseButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(hh);
    buttonLayout->addWidget(new QLabel(":"));
    buttonLayout->addWidget(mm);
    buttonLayout->addWidget(new QLabel(":"));
    buttonLayout->addWidget(ss);
    buttonLayout->addWidget(new QLabel("."));
    buttonLayout->addWidget(ms);

    mainLayout->addLayout(sliderLayout);
    mainLayout->addLayout(buttonLayout);

    connect(playPauseButton, &QPushButton::clicked, this, [this]{
       setPaused(!getPaused()); // toggle
       emit playPauseToggled(getPaused());
    });

    connect(rtsButton, &QPushButton::clicked, this, [this]{
        setPosition(0);
        emit returnToStartClicked();
    });

    connect(slider, &QSlider::sliderPressed, this, [this]{
        if(!paused) {
            emit playPauseToggled(true);
        }
    });

    connect(slider, &QSlider::sliderReleased, this, [this]{
        if(!paused) {
            emit playPauseToggled(false);
        }

        QTime time = QTime{0,0,0,0}.addMSecs(slider->value());
        hh->display(time.hour());
        mm->display(time.minute());
        ss->display(time.second());
        ms->display(time.msec());
        emit positionChangeRequested(slider->value());
    });

    setPaused(getPaused());

    setLayout(mainLayout);
}

bool TransportWidget::getPaused() const
{
    return paused;
}

void TransportWidget::setPaused(bool value)
{
    paused = value;
    if(paused) {
        playPauseButton->setIcon(QIcon{":/icons/play-solid.png"});
    } else {
        playPauseButton->setIcon(QIcon{":/icons/pause-solid.png"});
    }
}

void TransportWidget::setPosition(int milliseconds)
{
    QTime time = QTime{0,0,0,0}.addMSecs(milliseconds);
    hh->display(time.hour());
    mm->display(time.minute());
    ss->display(time.second());
    ms->display(time.msec());
    slider->setSliderPosition(milliseconds);
}

void TransportWidget::setLength(int milliseconds)
{
    slider->setMaximum(milliseconds);
}
