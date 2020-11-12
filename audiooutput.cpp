#include "audiooutput.h"



AudioOutputQueue::AudioOutputQueue(QObject *parent) : QObject(parent)
{
}

void AudioOutputQueue::setConfiguration(const QAudioDeviceInfo& audioDevice, const QAudioFormat& format)
{
    _output.reset(new QAudioOutput(audioDevice, format));

}

void AudioOutputQueue::addAudio(const QByteArray &buf)
{
    queue << buf;
}

void AudioOutputQueue::play()
{
    if(_output.get() != nullptr) {
        queue.setDevice(_output->start());
    }
}

