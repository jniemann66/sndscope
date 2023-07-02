#ifndef PLOTMODEWIDGET_H
#define PLOTMODEWIDGET_H

#include <QObject>
#include <QWidget>

#include <QComboBox>

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

signals:
	void plotmodeChanged(Plotmode plotmode);

private:
	QComboBox *plotmodeSelector{nullptr};

};

#endif // PLOTMODEWIDGET_H
