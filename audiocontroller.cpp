#include "audiocontroller.h"

#include <QDebug>

AudioController::AudioController(QObject *parent) : QObject(parent)
{

}

void AudioController::initializeAudio(const QAudioFormat &format, const QAudioDeviceInfo &deviceInfo)
{
    if (!deviceInfo.isFormatSupported(format)) {
        qWarning() << "Default format not supported - trying to use nearest";
    }

    audioOutput.reset(new QAudioOutput(deviceInfo, format));

//    qreal initialVolume = QAudio::convertVolume(audioOutput->volume(),
//                                                QAudio::LinearVolumeScale,
    //                                                QAudio::LogarithmicVolumeScale);

    audioOutput->setBufferSize(2 * format.sampleRate() * format.bytesPerFrame());

    connect(audioOutput.get(), &QAudioOutput::stateChanged, this, [this]{
        qDebug() << "state" << audioOutput->state();
    });

}

QIODevice* AudioController::start()
{
     return audioOutput->start();
}
