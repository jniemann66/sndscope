/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

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
	void setOutputVolume(qreal linearVolume);
	QIODevice *start();

signals:
	void outputVolume(qreal linearVol);

private:
	std::unique_ptr<QAudioOutput> audioOutput;
};

#endif // AUDIOCONTROLLER_H
