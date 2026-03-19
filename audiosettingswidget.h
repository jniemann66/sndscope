#ifndef AUDIOSETTINGSWIDGET_H
#define AUDIOSETTINGSWIDGET_H

#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QAudioDevice>
#include <QMediaDevices>

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
