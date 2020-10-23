#include "displaysettingswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

DisplaySettingsWidget::DisplaySettingsWidget(QWidget *parent) : QWidget(parent)
{
    brightnessControl = new QDial;
    focusControl = new QDial;

    auto mainLayout = new QVBoxLayout;
    auto sliderLayout = new QHBoxLayout;
    auto brightnessLayout = new QVBoxLayout;
    auto focusLayout = new QVBoxLayout;

    brightnessLayout->addWidget(new QLabel{"Brightness"});
    brightnessLayout->addWidget(brightnessControl);
    focusLayout->addWidget(new QLabel{"Focus"});
    focusLayout->addWidget(focusControl);
    sliderLayout->addLayout(brightnessLayout);
    sliderLayout->addLayout(focusLayout);

    mainLayout->addLayout(sliderLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);


    brightnessControl->setMaximum(1000);
    focusControl->setMaximum(1000);
    brightnessControl->setWrapping(false);
    focusControl->setWrapping(false);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    connect(brightnessControl, &QSlider::valueChanged, this, [this](int value){
        emit brightnessChanged(value * 0.1);
    });

    connect(focusControl, &QSlider::valueChanged, this, [this](int value){
       emit focusChanged((focusControl->maximum() - value) * 0.1);
    });
}

QSize DisplaySettingsWidget::sizeHint() const
{
    return {200, 320};
}

double DisplaySettingsWidget::getBrightness() const
{
    return 100.0 * brightnessControl->value() / brightnessControl->maximum();
}

void DisplaySettingsWidget::setBrightness(double value)
{
    brightnessControl->setValue(value * 0.01 * brightnessControl->maximum());
}

double DisplaySettingsWidget::getFocus() const
{
    return 100.0 * focusControl->value() / focusControl->maximum();
}

void DisplaySettingsWidget::setFocus(double value)
{
    focusControl->setValue((1.0 - value * 0.01) * focusControl->maximum());
}
