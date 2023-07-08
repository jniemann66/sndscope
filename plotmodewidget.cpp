#include "plotmodewidget.h"

#include <QVBoxLayout>
#include <QGroupBox>


PlotmodeWidget::PlotmodeWidget(QWidget *parent)
	: QWidget{parent}
{
	plotmodeSelector = new QComboBox;
	upsamplingCheckbox = new QCheckBox("upsampling");

	auto plotmodeLayout = new QHBoxLayout;
	auto mainLayout = new QVBoxLayout;
	plotmodeLayout->addWidget(plotmodeSelector);
	plotmodeLayout->addWidget(upsamplingCheckbox);


	for (const PlotmodeDefinition& p : PlotmodeManager::getPlotmodeMap())
	{
		plotmodeSelector->addItem(p.name, p.plotMode);
	}

	auto plotmodeBox = new QGroupBox("Plot Mode");
	plotmodeBox->setLayout(plotmodeLayout);

	mainLayout->addWidget(plotmodeBox);
	mainLayout->addStretch();
	setLayout(mainLayout);

	connect(plotmodeSelector,  QOverload<int>::of(&QComboBox::activated), this, [this](){
		emit plotmodeChanged(getPlotmode());
	});

	connect(upsamplingCheckbox, &QCheckBox::toggled, this, [this](){
		emit upsamplingChanged(upsamplingCheckbox->isChecked());
	});

}

Plotmode PlotmodeWidget::getPlotmode() const
{
	return plotmodeSelector->currentData(PlotmodeRole).value<Plotmode>();
}

void PlotmodeWidget::setPlotmode(Plotmode newPlotmode)
{
	for(int i = 0; i < plotmodeSelector->count(); i++) {
		if(plotmodeSelector->itemData(i, PlotmodeRole).value<Plotmode>() == newPlotmode) {
			plotmodeSelector->setCurrentIndex(i);
			break;
		}
	}
}
