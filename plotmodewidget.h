#ifndef PLOTMODEWIDGET_H
#define PLOTMODEWIDGET_H

#include <QObject>
#include <QWidget>

#include <QComboBox>
#include <QCheckBox>

#include "plotmode.h"

Q_DECLARE_METATYPE(Plotmode)

constexpr int PlotmodeRole = Qt::UserRole;

class PlotmodeWidget : public QWidget
{
	Q_OBJECT

public:
	explicit PlotmodeWidget(QWidget *parent = nullptr);

	Plotmode getPlotmode() const;

	void setPlotmode(Plotmode newPlotmode);
	void setconnectSamples(bool val);

signals:
	void plotmodeChanged(Plotmode plotmode);
	void upsamplingChanged(bool enableUpsampling);
	void connectSamplesChanged(bool enableconnectSamples);

private:
	QComboBox *plotmodeSelector{nullptr};
	QCheckBox *upsamplingCheckbox{nullptr};
	QCheckBox *connectSamples{nullptr};

};

#endif // PLOTMODEWIDGET_H
