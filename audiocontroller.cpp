#include "audiocontroller.h"

#include <QDebug>
#include <QMessageBox>

AudioController::AudioController(QObject *parent) : QObject(parent)
{

}

void AudioController::initializeAudio(const QAudioFormat &format, const QAudioDeviceInfo &deviceInfo)
{  
    static const QMap<QAudioFormat::SampleType, QString> sampleTypeMap
    {
        {QAudioFormat::Unknown, "Unknown"},
        {QAudioFormat::SignedInt, "Signed Int"},
        {QAudioFormat::UnSignedInt, "Unsigned Int"},
        {QAudioFormat::Float, "Floating Point"}
    };

    if (!deviceInfo.isFormatSupported(format)) {
        const QString msg = QStringLiteral("Audio Format Not Supported: %1Hz %2bit %3").arg(
                    QString::number(format.sampleRate()),
                    QString::number(format.sampleSize()),
                    sampleTypeMap.value(format.sampleType(), "Unknown")
                    );
        QMessageBox::warning(nullptr, "Audio Format Not Supported", msg);
        return;
    }

    audioOutput.reset(new QAudioOutput(deviceInfo, format));

//    qreal initialVolume = QAudio::convertVolume(audioOutput->volume(),
//                                                QAudio::LinearVolumeScale,
//                                                    QAudio::LogarithmicVolumeScale);

    audioOutput->setBufferSize(2 * format.sampleRate() * format.bytesPerFrame());

    connect(audioOutput.get(), &QAudioOutput::stateChanged, this, [this]{
        qDebug().noquote() << "Audio Status:" << audioOutput->state();
    });

}

QIODevice* AudioController::start()
{
     return audioOutput->start();
}
