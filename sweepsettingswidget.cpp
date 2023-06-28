#include "sweepsettingswidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

#include <cmath>

SweepSettingsWidget::SweepSettingsWidget(QWidget *parent)
	: QWidget{parent}
{
	auto sweepDialLabel = new QLabel("Sweep");
	sweepDial = new QSlider;
	sweepDial->setRange(-1, 21);
	sweepDial->setTickInterval(1);
	sweepDial->setOrientation(Qt::Orientation::Horizontal);
	sweepSpeedEdit = new QTextEdit;

	auto triggerLabel = new QLabel("Trigger");
	triggerLevel = new QSlider;
	triggerLevel->setRange(-32768, 32767);
	triggerLevel->setValue(0);

	auto slopeDialLabel = new QLabel("Slope: /");
	slopeDial = new QDial;
	slopeDial->setRange(-2, 2);
	slopeDial->setPageStep(1);
	slopeDial->setSingleStep(1);
	slopeDial->setSliderPosition(1);

	auto mainLayout = new QHBoxLayout;
	auto sweepSpeedLayout = new QVBoxLayout;
	auto triggerLayout = new QVBoxLayout;

	sweepSpeedLayout->addWidget(sweepDialLabel);
	sweepSpeedLayout->addWidget(sweepDial);
	sweepSpeedLayout->addWidget(sweepSpeedEdit);
	sweepSpeedLayout->addStretch();

	triggerLayout->addWidget(triggerLabel);
	triggerLayout->addWidget(triggerLevel);
	triggerLayout->addStretch();
	triggerLayout->addWidget(slopeDialLabel);
	triggerLayout->addWidget(slopeDial);
	triggerLayout->addStretch();

	mainLayout->addLayout(sweepSpeedLayout);
	mainLayout->addLayout(triggerLayout);

	setLayout(mainLayout);

	initSweepRateMap();

	connect(sweepDial, &QSlider::actionTriggered, this, [this]{
		sweepParameters.setDuration(sweepRateMap.value(std::max(0, sweepDial->value())));
		setSweepParametersText();
		emit sweepParametersChanged(sweepParameters);
	});

	auto setSlopeLabel = [slopeDialLabel](int s) {
		if(s < 0) {
			slopeDialLabel->setText("Slope: \\");
		} else {
			slopeDialLabel->setText("Slope: /");
		}
	};

	connect(slopeDial, &QDial::actionTriggered, this, [this]{
		auto v = slopeDial->sliderPosition() < 0 ? -1 : 1;
		slopeDial->setSliderPosition(v);
	});

	connect(slopeDial, &QDial::sliderMoved, this, [this]{
		auto v = slopeDial->sliderPosition() < 0 ? -1 : 1;
		slopeDial->setSliderPosition(v);
	});

	connect(slopeDial, &QDial::valueChanged, this, [this, setSlopeLabel](int v){
		sweepParameters.slope = (v < 0 ? -1.0 : 1.0);
		setSlopeLabel(v);
		emit sweepParametersChanged(sweepParameters);
	});

	connect(triggerLevel, &QSlider::valueChanged, this, [this](int value){
		sweepParameters.triggerLevel = value / (-1.0 * triggerLevel->minimum());
		emit sweepParametersChanged(sweepParameters);
	});

	connect(triggerLevel, &QSlider::sliderPressed, this, [this]{
		emit triggerLevelPressed(true);
	});

	connect(triggerLevel, &QSlider::sliderReleased, this, [this]{
		emit triggerLevelPressed(false);
	});

}

void SweepSettingsWidget::initSweepRateMap()
{
	static const QVector<double> m{5.0, 2.0, 1.0};
	const int c = m.size();
	for (int v = 0; v <= sweepDial->maximum(); v++) {
		sweepRateMap.insert(v, m.at(v % c) * std::pow(10.0, -(v / c)));
	}
}

SweepParameters SweepSettingsWidget::getSweepParameters() const
{
	return sweepParameters;
}

void SweepSettingsWidget::setSweepParameters(const SweepParameters &newSweepParameters)
{
	sweepParameters = newSweepParameters;
	double d = sweepParameters.getDuration();
	for(int v = 0; v < sweepRateMap.size(); v++) {
		if(d >= sweepRateMap.value(v)) {
			sweepDial->setValue(v);
			break;
		}
	}
	setSweepParametersText();
}

void SweepSettingsWidget::setSweepParametersText()
{
	double d = sweepParameters.getDuration();
	sweepSpeedEdit->setText(QStringLiteral(
								"%4 Hz <br/>"
								"%1s / sweep<br/> "
										   "%2s / div<br/>"
										   "%3 smpl / div<br/>"
										   )
							.arg(d)
							.arg(d / 5)
							.arg(sweepParameters.getSamplesPerSweep() / 5)
							.arg(1.0 / sweepParameters.getDuration())
							);
}
