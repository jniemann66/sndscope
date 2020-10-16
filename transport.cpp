#include "transport.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>

Transport::Transport(QWidget *parent) : QWidget(parent)
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

    setLayout(mainLayout);
}

bool Transport::getPaused() const
{
    return paused;
}

void Transport::setPaused(bool value)
{
    paused = value;
}

void Transport::setPosition(int milliseconds)
{
    QTime time = QTime{0, 0, 0, 0}.addMSecs(milliseconds);
    hh->display(time.hour());
    mm->display(time.minute());
    ss->display(time.second());
    ms->display(time.msec());
    slider->setSliderPosition(milliseconds);
}

void Transport::setLength(int milliseconds)
{
    slider->setMaximum(milliseconds);
}
