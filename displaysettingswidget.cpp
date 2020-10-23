#include "displaysettingswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

DisplaySettingsWidget::DisplaySettingsWidget(QWidget *parent) : QWidget(parent)
{
    brightnessSlider = new QSlider(Qt::Vertical);
    focusSlider = new QSlider(Qt::Vertical);

    auto mainLayout = new QVBoxLayout;
    auto sliderLayout = new QHBoxLayout;
    auto brightnessLayout = new QVBoxLayout;
    auto focusLayout = new QVBoxLayout;

    brightnessLayout->addWidget(new QLabel{"Brightness"});
    brightnessLayout->addWidget(brightnessSlider);
    focusLayout->addWidget(new QLabel{"Focus"});
    focusLayout->addWidget(focusSlider);
    sliderLayout->addLayout(brightnessLayout);
    sliderLayout->addLayout(focusLayout);

    mainLayout->addLayout(sliderLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);


    brightnessSlider->setMaximum(1000);
    focusSlider->setMaximum(1000);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    connect(brightnessSlider, &QSlider::valueChanged, this, [this](int value){
        emit brightnessChanged(value * 0.1);
    });

    connect(focusSlider, &QSlider::valueChanged, this, [this](int value){
       emit focusChanged((focusSlider->maximum() - value) * 0.1);
    });
}

QSize DisplaySettingsWidget::sizeHint() const
{
    return {200, 320};
}

double DisplaySettingsWidget::getBrightness() const
{
    return 100.0 * brightnessSlider->value() / brightnessSlider->maximum();
}

void DisplaySettingsWidget::setBrightness(double value)
{
    brightnessSlider->setValue(value * 0.01 * brightnessSlider->maximum());
}

double DisplaySettingsWidget::getFocus() const
{
    return 100.0 * focusSlider->value() / focusSlider->maximum();
}

void DisplaySettingsWidget::setFocus(double value)
{
    focusSlider->setValue((1.0 - value * 0.01) * focusSlider->maximum());
}
