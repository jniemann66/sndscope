#include "displaysettingswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFile>

DisplaySettingsWidget::DisplaySettingsWidget(QWidget *parent) : QWidget(parent)
{
    brightnessControl = new QDial;
    focusControl = new QDial;
    phosphorSelectControl = new QComboBox;
    persistenceControl = new QDial;
    clearScreenButton = new QPushButton;

    clearScreenButton->setIcon(QIcon{":/icons/wipe-small.png"});
    clearScreenButton->setIconSize({48, 48});
    clearScreenButton->setToolTip("Wipe Screen");

    // set properties of dial controls
    brightnessControl->setMaximum(1000);
    focusControl->setMaximum(1000);
    persistenceControl->setMaximum(1500);
    brightnessControl->setNotchesVisible(true);
    brightnessControl->setNotchTarget(100);
    focusControl->setNotchesVisible(true);
    focusControl->setNotchTarget(100);
    persistenceControl->setNotchesVisible(true);
    persistenceControl->setNotchTarget(100);

    brightnessControl->setWrapping(false);
    focusControl->setWrapping(false);
    persistenceControl->setWrapping(false);

    auto mainLayout = new QVBoxLayout;
    auto controlLayout1 = new QHBoxLayout;
    auto controlLayout2 = new QHBoxLayout;
    auto brightnessLayout = new QVBoxLayout;
    auto focusLayout = new QVBoxLayout;
    auto phosphorSelectLayout = new QVBoxLayout;
    auto persistenceLayout = new QVBoxLayout;

    brightnessLayout->addWidget(new QLabel{"Brightness"});
    brightnessLayout->addWidget(brightnessControl);
    focusLayout->addWidget(new QLabel{"Focus"});
    focusLayout->addWidget(focusControl);
    phosphorSelectLayout->addWidget(new QLabel("Phosphor"));
    phosphorSelectLayout->addWidget(phosphorSelectControl);
    phosphorSelectLayout->addWidget(clearScreenButton);
    phosphorSelectLayout->setAlignment(Qt::AlignTop);
    persistenceLayout->addWidget(new QLabel{"Persistence"});
    persistenceLayout->addWidget(persistenceControl);

    controlLayout1->addLayout(brightnessLayout);
    controlLayout1->addLayout(focusLayout);

    controlLayout2->addLayout(phosphorSelectLayout);
    controlLayout2->addLayout(persistenceLayout);

    mainLayout->addLayout(controlLayout1);
    mainLayout->addLayout(controlLayout2);
    mainLayout->addStretch();
    setLayout(mainLayout);



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

    connect(phosphorSelectControl, &QComboBox::activated, this, [this]{
       QString name = phosphorSelectControl->currentText();
       if(phosphors.contains(name)) {
           Phosphor phosphor = phosphors.value(name);
           QVector<QColor> phosphorColors;
           for (int i = 0; i < phosphor.layers.count(); i++) {
               const auto& c = phosphor.layers.at(i).color;
               QColor phosphorColor{c.red(), c.green(), c.blue(), 255};
               phosphorColors.append(c);
           }

           if(phosphorColors.count() > 0) {
               setPersistence(phosphor.layers.at(0).persistence);
               emit phosphorColorChanged({phosphorColors.at(0)});
               emit persistenceChanged(phosphor.layers.at(0).persistence);
               if(phosphorColors.count() > 1) {
                   emit multiColorPhosphorChanged(true, phosphorColors.at(1));
               } else {
                   emit multiColorPhosphorChanged(false, QColor{0, 0, 0, 0});
               }
           }
       }
    });

    connect(clearScreenButton, &QPushButton::pressed, this, [this]{
        emit wipeScreenRequested();
    });

    auto _r  = loadPhosphors(":/phosphors.json");
    if(_r.first) {
        phosphorSelectControl->clear();
        for(const QString& phosphorName : phosphors.keys()) {
            phosphorSelectControl->addItem(phosphorName);
        }
        phosphorSelectControl->setCurrentText("P31");
    }

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

QPair<bool, QString> DisplaySettingsWidget::loadPhosphors(const QString& filename)
{
    QFile f(filename);
    if(f.open(QFile::ReadOnly)) {
        QJsonParseError e;
        auto doc = QJsonDocument::fromJson(f.readAll(), &e);
        if(e.error != QJsonParseError::NoError) {
            return {false, e.errorString()};
        }
        phosphors.clear();
        QJsonObject o = doc.object();
        if(o.value("phosphors").isArray()) {
            auto a = o.value("phosphors").toArray();
            for(int i = 0; i < a.count(); i++) {
                if(a.at(i).isObject()) {
                    Phosphor phosphor;
                    phosphor.fromJson(a.at(i).toObject());
                    phosphors.insert(phosphor.name, phosphor);
                }
            }
            return {!phosphors.isEmpty(), ""};
        }
    }

    return {false, QString{"Couldn't open '%1'"}.arg(filename)};

}
