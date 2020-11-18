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



void AudioOutputQueue::addAudio(float val)
{   
    queue << val;
}

void AudioOutputQueue::play()
{
    if(_output.get() != nullptr) {
        queue.setDevice(_output->start());
    }
}

int64_t AudioOutputQueue::size() const
{
    if(queue.device() != nullptr) {
        return queue.device()->size();
    }
    return 0;
}

