/*
* Copyright (C) 2020 - 2021 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

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
