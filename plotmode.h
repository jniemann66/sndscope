#ifndef PLOTMODE_H
#define PLOTMODE_H

#include <QString>
#include <QMap>

enum Plotmode
{
	XY,
	MidSide,
	Sweep,
	SweepUpsampled
};



struct PlotmodeDefinition
{
	Plotmode plotMode;
	QString name;
	QString description;
};

class PlotmodeManager
{
	static QMap<Plotmode, PlotmodeDefinition> plotmodeMap;

public:
	static const QMap<Plotmode, PlotmodeDefinition>& getPlotmodeMap();
};


#endif // PLOTMODE_H
