/*
* Copyright (C) 2020 - 2021 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QAudioDevice>
#include <QAudioSink>
#include <QDataStream>
#include <QObject>

#include <memory>

class AudioOutputQueue : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutputQueue(QObject *parent = nullptr);
    void setConfiguration(const QAudioDevice &audioDevice, const QAudioFormat &format = QAudioFormat());
    void addAudio(float val);
    void play();
    void stop();
    int64_t size() const;


signals:

private:
    std::unique_ptr<QAudioSink> _output;
    QAudioDevice audioDeviceInfo;
    QDataStream datastream;
    QByteArray buffer;
};

#endif // AUDIOOUTPUT_H
