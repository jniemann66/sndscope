#include "displaysettingswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

DisplaySettingsWidget::DisplaySettingsWidget(QWidget *parent) : QWidget(parent)
{
    brightnessControl = new QDial;
    focusControl = new QDial;
    persistenceControl = new QDial;

    auto mainLayout = new QVBoxLayout;
    auto controlLayout1 = new QHBoxLayout;
    auto controlLayout2 = new QHBoxLayout;
    auto brightnessLayout = new QVBoxLayout;
    auto focusLayout = new QVBoxLayout;
    auto persistenceLayout = new QVBoxLayout;

    brightnessLayout->addWidget(new QLabel{"Brightness"});
    brightnessLayout->addWidget(brightnessControl);
    focusLayout->addWidget(new QLabel{"Focus"});
    focusLayout->addWidget(focusControl);
    persistenceLayout->addWidget(new QLabel{"Persistence"});
    persistenceLayout->addWidget(persistenceControl);
    controlLayout1->addLayout(brightnessLayout);
    controlLayout1->addLayout(focusLayout);

    controlLayout2->addStretch();
    controlLayout2->addLayout(persistenceLayout);

    mainLayout->addLayout(controlLayout1);
    mainLayout->addLayout(controlLayout2);
    mainLayout->addStretch();
    setLayout(mainLayout);

    brightnessControl->setMaximum(1000);
    focusControl->setMaximum(1000);
    persistenceControl->setMaximum(1500);

    brightnessControl->setWrapping(false);
    focusControl->setWrapping(false);
    persistenceControl->setWrapping(false);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    connect(brightnessControl, &QSlider::valueChanged, this, [this](int value){
        emit brightnessChanged(value * 0.1);
    });

    connect(focusControl, &QSlider::valueChanged, this, [this](int value){
       emit focusChanged((focusControl->maximum() - value) * 0.1);
    });

    connect(persistenceControl, &QDial::valueChanged, this, [this](int value){
       emit persistenceChanged(value);
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

int DisplaySettingsWidget::getPersistence() const
{
    return persistenceControl->value();
}

void DisplaySettingsWidget::setPersistence(int value)
{
    persistenceControl->setValue(value);
}
