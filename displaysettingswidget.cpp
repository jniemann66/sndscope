/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#include "displaysettingswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFile>
#include <QGroupBox>

DisplaySettingsWidget::DisplaySettingsWidget(QWidget *parent) : QWidget(parent)
{
	brightnessControl = new QDial;
	focusControl = new QDial;
	phosphorSelectControl = new QComboBox;
	persistenceControl = new QDial;
	clearScreenButton = new QPushButton;

	auto mainLayout = new QVBoxLayout;
	auto controlLayout1 = new QHBoxLayout;
	auto controlLayout2 = new QHBoxLayout;
	auto brightnessLayout = new QVBoxLayout;
	auto focusLayout = new QVBoxLayout;
	auto phosphorSelectLayout = new QVBoxLayout;
	auto persistenceLayout = new QVBoxLayout;
	auto beamGroupBox = new QGroupBox("Beam");
	auto phosphorGroupBox = new QGroupBox("Phosphor");

	// set widget properties
	brightnessControl->setMaximum(1000);
	focusControl->setMaximum(1000);
	persistenceControl->setMaximum(1500);
	brightnessControl->setNotchesVisible(true);
	brightnessControl->setNotchTarget(100);
	brightnessControl->setWrapping(false);
	focusControl->setNotchesVisible(true);
	focusControl->setNotchTarget(100);
	persistenceControl->setNotchesVisible(true);
	persistenceControl->setNotchTarget(100);
	brightnessControl->setWrapping(false);
	focusControl->setWrapping(false);
	persistenceControl->setWrapping(false);
//	clearScreenButton->setIcon(QIcon{":/icons/wipe-small.png"});
	clearScreenButton->setText("Clear Screen");
//	clearScreenButton->setIconSize({48, 48});
	clearScreenButton->setToolTip("Wipe Screen");

	// organize layouts
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
	beamGroupBox->setLayout(controlLayout1);
	phosphorGroupBox->setLayout(controlLayout2);
	mainLayout->addWidget(beamGroupBox);
	mainLayout->addWidget(phosphorGroupBox);
	mainLayout->addStretch();
	setLayout(mainLayout);

	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	connect(brightnessControl, &QSlider::valueChanged, this, [this](){
		emit brightnessChanged(getBrightness());
	});

	connect(focusControl, &QSlider::valueChanged, this, [this](){
		emit focusChanged(getFocus());
	});

	connect(persistenceControl, &QDial::valueChanged, this, [this](){
		emit persistenceChanged(getPersistence());
	});

	connect(phosphorSelectControl, &QComboBox::currentTextChanged, this, [this](const QString& name){

		if(phosphors.contains(name)) {
			Phosphor phosphor = phosphors.value(name);
			QVector<QColor> phosphorColors;
			for (int i = 0; i < phosphor.layers.count(); i++) {
				const auto& c = phosphor.layers.at(i).color;
				phosphorColors.append(c);
			}

			if(phosphorColors.count() > 0) {
				emit phosphorColorChanged(phosphorColors);
				setPersistence(phosphor.layers.at(0).persistence);
			}
		}
	});

	connect(clearScreenButton, &QPushButton::pressed, this, [this]{
		emit wipeScreenRequested();
	});

	auto _r  = loadPhosphors(":/phosphors.json");
	if(_r.first) {
		phosphorSelectControl->clear();
		const QStringList phosphorNames = phosphors.keys();
		for(const QString& phosphorName : phosphorNames) {
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
	focusControl->setValue(value * 0.01 * focusControl->maximum());
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
