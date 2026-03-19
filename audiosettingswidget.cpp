#include "audiosettingswidget.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

AudioSettingsWidget::AudioSettingsWidget(QWidget *parent)
	: QWidget{parent}
{
	auto deviceSelectorLabel = new QLabel("Output Device");
	deviceSelector = new QComboBox;
	auto volumeSliderLabel = new QLabel;

	volumeSliderLabel->setPixmap(QPixmap{":/icons/speaker1.png"}.scaled({24,24}));
	volumeSlider = new QSlider;
	volumeSlider->setRange(0, 100);
	volumeSlider->setValue(100);
	volumeSlider->setTickInterval(10);

	auto outputDeviceLayout = new QVBoxLayout;
	outputDeviceLayout->addWidget(deviceSelectorLabel);
	outputDeviceLayout->addWidget(deviceSelector);
	outputDeviceLayout->addStretch();

	auto volumeLayout = new QVBoxLayout;
	volumeLayout->addWidget(volumeSliderLabel);
	volumeLayout->addWidget(volumeSlider);

	auto mainLayout = new QHBoxLayout;
	mainLayout->addLayout(outputDeviceLayout);
	mainLayout->addLayout(volumeLayout);
	setLayout(mainLayout);

	setAvailableOutputDevices(QMediaDevices::audioOutputs());

	connect(deviceSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]{
		const auto d  = deviceSelector->currentData().value<QAudioDevice>();
		emit outputDeviceSelected(d);
	});

	connect(volumeSlider, &QSlider::sliderMoved, this, [this](int position){

		emit outputVolumeChanged(QAudio::convertVolume (position / 100.0, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale));
	});
}

void AudioSettingsWidget::setAvailableOutputDevices(const QList<QAudioDevice> &deviceList)
{
	deviceSelector->clear();
	for(auto it = deviceList.constBegin(); it != deviceList.constEnd(); ++it)
	{
		QVariant v;
		v.setValue(*it);
		deviceSelector->addItem(it->description(), v);
	}
	deviceSelector->setCurrentText(QMediaDevices::defaultAudioOutput().description());
}

void AudioSettingsWidget::setVolume(qreal linearVol)
{
	volumeSlider->setSliderPosition(100  * QAudio::convertVolume(linearVol,
																 QAudio::LinearVolumeScale,
																 QAudio::LogarithmicVolumeScale));

}

QAudioDevice AudioSettingsWidget::getSelectedAudioDevice() const
{
	return deviceSelector->currentData().value<QAudioDevice>();
}
