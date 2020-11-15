#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QAudioOutput>
#include <memory>
#include <QDataStream>

class AudioOutputQueue : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutputQueue(QObject *parent = nullptr);
    void setConfiguration(const QAudioDeviceInfo &audioDevice, const QAudioFormat &format = QAudioFormat());
    void addAudio(const QByteArray& buf);
    void addAudio(const QVector<double> &buf);
    void play();
    void stop();


signals:

private:
    std::unique_ptr<QAudioOutput> _output;
    QAudioDeviceInfo audioDeviceInfo;
    QDataStream queue;
};

#endif // AUDIOOUTPUT_H
