#include "audiooutput.h"
#include <QDebug>

AudioOutputQueue::AudioOutputQueue(QObject *parent) : QObject(parent), datastream(&buffer, QIODevice::WriteOnly)
{
    datastream.setFloatingPointPrecision(QDataStream::SinglePrecision);
}

void AudioOutputQueue::setConfiguration(const QAudioDeviceInfo& audioDevice, const QAudioFormat& format)
{
    _output.reset(new QAudioOutput(audioDevice, format));
    _output->setVolume(1.0);
    qDebug() << _output->state() << _output->error();
}

void AudioOutputQueue::addAudio(float val)
{   
    datastream << val;
}

void AudioOutputQueue::play()
{
    if(_output.get() != nullptr) {
        _output->start(datastream.device());
    }
}

int64_t AudioOutputQueue::size() const
{
    if(datastream.device() != nullptr) {
        return datastream.device()->size();
    }
    return 0;
}
