#include "plotmode.h"

QMap<Plotmode, PlotmodeDefinition> PlotmodeManager::plotmodeMap
{
	{XY, {XY, "X / Y", "X Axis: Ch0<br/>Y Axis: Ch1"}},
	{MidSide, {MidSide, "Mid / Side", "X Axis: Ch0 - Ch1<br/>Y Axis: Ch0 + Ch1"}},
	{Sweep, {Sweep, "Sweep", "X Axis: Sweep<br/>Y Axis: ch0"}}
};

const QMap<Plotmode, PlotmodeDefinition>& PlotmodeManager::getPlotmodeMap()
{
	return plotmodeMap;
}
