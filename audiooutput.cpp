#include "audiooutput.h"
#include <QDebug>

AudioOutputQueue::AudioOutputQueue(QObject *parent) : QObject(parent)
{
}

void AudioOutputQueue::setConfiguration(const QAudioDeviceInfo& audioDevice, const QAudioFormat& format)
{
    _output.reset(new QAudioOutput(audioDevice, format));
    _output->setVolume(1.0);
    qDebug() << _output->state() << _output->error();

}

void AudioOutputQueue::addAudio(const QByteArray &buf)
{
    queue << buf;
}

void AudioOutputQueue::addAudio(const QVector<double> &buf)
{
    for(int i  = 0; i < buf.length(); i++) {
        queue << static_cast<float>(buf.at(i));
    }
}

void AudioOutputQueue::play()
{
    if(_output.get() != nullptr) {
        queue.setDevice(_output->start());
    }
}

