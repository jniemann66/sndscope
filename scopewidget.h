/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef SCOPEWIDGET_H
#define SCOPEWIDGET_H

#include <memory>

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QTime>
#include <QColor>
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>
#include <QPixmap>

#include <sndfile.hh>

#include "audiocontroller.h"

class PictureBox : public QLabel
{
    Q_OBJECT
public:
    PictureBox(QPixmap* pixmap, QWidget* parent = nullptr) : QLabel(parent), pixmap(pixmap)
    {
        setPixmap(*pixmap);
        setScaledContents(true);
        resizeCooldown.setSingleShot(true);
        resizeCooldown.setInterval(200);

        connect(&resizeCooldown, &QTimer::timeout, this, [this]{
            const int h = height();
            qDebug().noquote() << QStringLiteral("adjusting pixmap resolution to %1x%1").arg(h);
            *PictureBox::pixmap = PictureBox::pixmap->scaledToHeight(h);
            emit pixmapResolutionChanged(PictureBox::pixmap->size());
        });
    }

    void setAllowPixmapResolutionChange(bool value);

signals:
    void pixmapResolutionChanged(const QSizeF& size);

protected:
    void resizeEvent(QResizeEvent *event) override {
        auto h = event->size().height();
        auto w = event->size().width();
        if(w != h) {
            setFixedWidth(h);
            if(allowPixmapResolutionChange) {
                resizeCooldown.start();
            }
        }
    }

private:
    QTimer resizeCooldown;
    QPixmap *pixmap{nullptr};
    bool allowPixmapResolutionChange{true};
};

class ScopeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScopeWidget(QWidget *parent = nullptr);
    QPair<bool, QString> loadSoundFile(const QString &filename);
    int getLengthMilliseconds();
    bool getPaused() const;
    void setPaused(bool value);

    int64_t getTotalFrames() const;
    void setTotalFrames(const int64_t &value);

    double getBrightness() const;
    void setBrightness(double value);

    double getFocus() const;
    void setFocus(double value);

    QRgb getPhosphorColor() const;
    void setPhosphorColor(const QRgb &value);

    double getPersistence() const;
    void setPersistence(double value);

    bool getMultiColor() const;
    void setMultiColor(bool value, const QColor& altColor);

    QColor getBackgroundColor() const;
    void setBackgroundColor(const QColor &value);

public slots:
    void returnToStart();
    void gotoPosition(int64_t milliSeconds);
    void wipeScreen();

signals:
    void renderedFrame(int positionMilliseconds);

protected:

private:
    PictureBox* screenWidget;
    std::unique_ptr<SndfileHandle> sndfile;
    QVector<float> inputBuffer;
    QVector<float> audioOutBuffer;
    QIODevice* pushOut{nullptr};
    AudioController *audioController{nullptr};
    QAudioFormat audioFormat;

    QPixmap pixmap;

    // midpoint of pixmap
    double cx;
    double cy;

    QTimer plotTimer;
    QElapsedTimer elapsedTimer;
    int framesPerMillisecond{0};
    double millisecondsPerFrame{0.0};
    int64_t startFrame{0};
    int64_t currentFrame{0ll};
    int64_t maxFramesToRead{0};
    int64_t totalFrames{0ll};
    bool fileLoaded{false};
    bool paused{true};
    int screenDrawCounter{0};
    double brightness;
    double focus;
    int darkenAlpha;
    int darkenNthFrame{1};
    int darkenCooldownCounter{1};
    int beamAlpha;
    QRgb phosphorColor{0xff3eff6f};
    QColor darkencolor{0, 0, 0, 0};
    QColor backgroundColor{0, 0, 0, 0};
    bool multiColor{false};
    QPainter::CompositionMode compositionMode{QPainter::CompositionMode_SourceOver};
    double persistence;
    double beamWidth;
    double beamIntensity;

    void calcCenter();
    void render();
    void calcBeamAlpha();

signals:

};

#endif // SCOPEWIDGET_H
