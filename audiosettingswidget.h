#ifndef AUDIOSETTINGSWIDGET_H
#define AUDIOSETTINGSWIDGET_H

#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QAudioDeviceInfo>

class AudioSettingsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit AudioSettingsWidget(QWidget *parent = nullptr);
	void setAvailableOutputDevices(const QList<QAudioDeviceInfo> &deviceList);
	void setVolume(qreal linearVol);
	QAudioDeviceInfo getSelectedAudioDevice() const;

signals:
	void outputDeviceSelected(const QAudioDeviceInfo &device);
	void outputVolumeChanged(qreal linearVolume);

private:
	QComboBox *deviceSelector{nullptr};
	QSlider *volumeSlider;
};

#endif // AUDIOSETTINGSWIDGET_H
