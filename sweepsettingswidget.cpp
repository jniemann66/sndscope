#include "sweepsettingswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTimer>

#include <QDebug>

#include <cmath>

SweepSettingsWidget::SweepSettingsWidget(QWidget *parent)
	: QWidget{parent}
{
	auto sweepDialLabel = new QLabel("Rate");
	sweepDial = new QSlider;
	sweepDial->setRange(-1, 21);
	sweepDial->setTickInterval(1);
	sweepDial->setOrientation(Qt::Orientation::Horizontal);
	sweepInfo = new QLabel;

	auto triggerLevelLabel = new QLabel("Level");
	triggerLevel = new QSlider;
	triggerLevel->setRange(-32768, 32767);
	triggerLevel->setValue(0);

	auto triggerToleranceLabel = new QLabel("Tolerance");
	triggerTolerance = new QSlider;
	triggerTolerance->setRange(1, 100);
	triggerTolerance->setValue(10);

	triggerEnabled = new QCheckBox("Enabled");
	triggerEnabled->setChecked(true);
	triggerResetButton = new QCheckBox("Reset");



	auto slopeDialLabel = new QLabel("Slope: /");
	slopeDial = new QDial;
	slopeDial->setRange(-2, 2);
	slopeDial->setPageStep(1);
	slopeDial->setSingleStep(1);
	slopeDial->setSliderPosition(1);

	auto mainLayout = new QVBoxLayout;
	auto sweepLayout = new QVBoxLayout;
	auto sweepSpeedLayout = new QVBoxLayout;
	auto sweepInfoLayout = new QHBoxLayout;
	auto triggerLevelLayout = new QVBoxLayout;
	auto triggerToleranceLayout = new QVBoxLayout;
	auto triggerResetLayout = new QVBoxLayout;

	auto triggerSlopeLayout = new QVBoxLayout;
	auto triggerLayout = new QHBoxLayout;



	sweepSpeedLayout->addWidget(sweepDialLabel);
	sweepSpeedLayout->addWidget(sweepDial);

	sweepInfoLayout->addWidget(sweepInfo);

	sweepLayout->addLayout(sweepSpeedLayout);
	sweepLayout->addLayout(sweepInfoLayout);

	triggerLevelLayout->addWidget(triggerLevelLabel);
	triggerLevelLayout->addWidget(triggerLevel);

	triggerToleranceLayout->addWidget(triggerToleranceLabel);
	triggerToleranceLayout->addWidget(triggerTolerance);

	triggerSlopeLayout->addWidget(slopeDialLabel);
	triggerSlopeLayout->addWidget(slopeDial);
	triggerSlopeLayout->addStretch();

	//triggerResetLayout->addStretch();
	triggerResetLayout->addWidget(triggerEnabled);
	triggerResetLayout->addWidget(triggerResetButton);
	triggerResetLayout->addStretch();

	triggerLayout->addLayout(triggerResetLayout, 1);
	triggerLayout->addLayout(triggerLevelLayout, 2);
	triggerLayout->addLayout(triggerToleranceLayout, 2);
	triggerLayout->addLayout(triggerSlopeLayout,2);

	auto sweepBox = new QGroupBox("Sweep");
	auto triggerBox = new QGroupBox("Trigger");

	sweepBox->setLayout(sweepLayout);
	triggerBox->setLayout(triggerLayout);

	mainLayout->addWidget(sweepBox);
	mainLayout->addWidget(triggerBox);

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

	connect(triggerEnabled, &QCheckBox::stateChanged, this, [this](int state){
		triggerResetButton->setEnabled(state == Qt::Checked);
		triggerLevel->setEnabled(state == Qt::Checked);
		triggerTolerance->setEnabled(state == Qt::Checked);
		slopeDial->setEnabled(state == Qt::Checked);
		sweepParameters.triggerEnabled = (state == Qt::Checked);
		emit sweepParametersChanged(sweepParameters);
	});

	connect(triggerResetButton, &QCheckBox::clicked, this, [this]{
		triggerLevel->setValue(0);
		triggerTolerance->setValue(10);
		slopeDial->setValue(1.0);

		sweepParameters.triggerLevel = 0.0;
		sweepParameters.triggerTolerance = 0.01;
		sweepParameters.slope = 1.0;

		QTimer::singleShot(150, this, [this]{
			triggerResetButton->setChecked(false);
		});

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

	connect(triggerTolerance, &QSlider::valueChanged, this, [this](int value){
		sweepParameters.triggerTolerance = 0.001 * value;
		emit sweepParametersChanged(sweepParameters);
	});

	connect(triggerTolerance, &QSlider::sliderPressed, this, [this]{
		emit triggerLevelPressed(true);
	});

	connect(triggerTolerance, &QSlider::sliderReleased, this, [this]{
		emit triggerLevelPressed(false);
	});

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

	//triggerEnabled->setChecked(!sweepParameters.sweepUnused);

	setSweepParametersText();
}

void SweepSettingsWidget::setSweepParametersText()
{
	double d = sweepParameters.getDuration();
	sweepInfo->setText(QStringLiteral(
								"<b>%1</b> <br/>"
								"<b>%2</b> / sweep<br/> "
								"<b>%3</b> / div<br/>"
								"<b>%4</b> smpl / div<br/>"
								)
							.arg(SweepParameters::formatMeasurementUnits(1.0 / sweepParameters.getDuration(), "Hz", 0),
								 SweepParameters::formatMeasurementUnits(d, "s", 0),
								 SweepParameters::formatMeasurementUnits(d / std::max<double>(1.0, sweepParameters.horizontalDivisions), "s", 2))
							.arg(sweepParameters.getSamplesPerSweep() / std::max<double>(1.0, sweepParameters.horizontalDivisions))
							);
}
