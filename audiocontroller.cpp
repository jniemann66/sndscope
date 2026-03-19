/*
* Copyright (C) 2020 - 2026 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#include "audiocontroller.h"

#include <QDebug>
#include <QMessageBox>

AudioController::AudioController(QObject *parent) : QObject(parent)
{
}

void AudioController::initializeAudio(const QAudioFormat &format, const QAudioDevice &deviceInfo)
{
	static const QMap<QAudioFormat::SampleFormat, QString> sampleTypeMap
	{
		{QAudioFormat::Unknown, "Unknown"},
		{QAudioFormat::Int16, "Signed Int 16"},
		{QAudioFormat::Int32, "Signed Int 32"},
		{QAudioFormat::UInt8, "Unsigned Int 8"},
		{QAudioFormat::Float, "Floating Point"}
	};

	if (!deviceInfo.isFormatSupported(format)) {
		const QString msg = QStringLiteral("Audio Format Not Supported: %1Hz %2bit %3")
				.arg(QString::number(format.sampleRate()),
					 QString::number(format.bytesPerSample() * 8),
					 sampleTypeMap.value(format.sampleFormat(), "Unknown"));
		QMessageBox::warning(nullptr, "Audio Format Not Supported", msg);
		return;
	}

	audioOutput.reset(new QAudioSink(deviceInfo, format));
	emit outputVolume(audioOutput->volume());

	audioOutput->setBufferSize(2 * format.sampleRate() * format.bytesPerFrame());

	connect(audioOutput.get(), &QAudioSink::stateChanged, this, [this]{
		qDebug().noquote() << "Audio Status:" << audioOutput->state();
	});

}

void AudioController::setOutputVolume(qreal linearVolume)
{
	if(audioOutput != nullptr) {
		audioOutput->setVolume(linearVolume);
	}
}

QIODevice* AudioController::start()
{
	return audioOutput->start();
}
