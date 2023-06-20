#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include <memory>

#include <QObject>
#include <QAudioDeviceInfo>
#include <QAudioOutput>

class AudioController : public QObject
{
	Q_OBJECT
public:
	explicit AudioController(QObject *parent = nullptr);
	void initializeAudio(const QAudioFormat &format, const QAudioDeviceInfo &deviceInfo);
	QIODevice *start();

signals:

private:
	std::unique_ptr<QAudioOutput> audioOutput;
};

#endif // AUDIOCONTROLLER_H
