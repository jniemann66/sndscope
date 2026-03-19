#ifndef AUDIOSETTINGSWIDGET_H
#define AUDIOSETTINGSWIDGET_H

#include <QAudioDevice>
#include <QComboBox>
#include <QMediaDevices>
#include <QObject>
#include <QSlider>
#include <QWidget>

class AudioSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	explicit AudioSettingsWidget(QWidget *parent = nullptr);
	void setAvailableOutputDevices(const QList<QAudioDevice> &deviceList);
	void setVolume(qreal linearVol);
	QAudioDevice getSelectedAudioDevice() const;

signals:
	void outputDeviceSelected(const QAudioDevice &device);
	void outputVolumeChanged(qreal linearVolume);

private:
	QComboBox *deviceSelector{nullptr};
	QSlider *volumeSlider;
};

#endif // AUDIOSETTINGSWIDGET_H
