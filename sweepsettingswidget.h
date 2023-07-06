#ifndef SWEEPSETTINGSWIDGET_H
#define SWEEPSETTINGSWIDGET_H

#include <QObject>
#include <QWidget>

#include <QDial>
#include <QSlider>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QMap>
#include <QPushButton>

#include "sweepparameters.h"

class SweepSettingsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit SweepSettingsWidget(QWidget *parent = nullptr);

	SweepParameters getSweepParameters() const;
	void setSweepParameters(const SweepParameters &newSweepParameters);

signals:
	void sweepParametersChanged(const SweepParameters& sweepParameters);
	void triggerLevelPressed(bool isPressed);

private:
	QMap<int, double> sweepRateMap;

	QSlider *sweepDial{nullptr};
	QLabel *sweepInfo{nullptr};
	QSlider *triggerLevel{nullptr};
	QSlider *triggerTolerance{nullptr};
	QCheckBox *triggerEnabled{nullptr};
	QCheckBox *triggerResetButton{nullptr};
	QCheckBox *connectDots{nullptr};
	QDial *slopeDial{nullptr};

	void initSweepRateMap();

	SweepParameters sweepParameters;
	void setSweepParametersText();
};

#endif // SWEEPSETTINGSWIDGET_H
