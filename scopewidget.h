/*
* Copyright (C) 2020 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope
*/

#ifndef SCOPEWIDGET_H
#define SCOPEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QTime>

#include <memory>

#include <sndfile.hh>

class SizeTracker : public QObject
{
    Q_OBJECT
public:

    SizeTracker(QObject* parent);
    double cx;
    double cy;
    double w;
    double h;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
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

public slots:
    void returnToStart();
    void gotoPosition(int64_t milliSeconds);

signals:
    void renderedFrame(int positionMilliseconds);

protected:

private:
    QLabel* screenWidget;
    std::unique_ptr<SndfileHandle> h;
    QVector<double> inputBuffer;
    SizeTracker* sizeTracker;

    QPixmap pixmap;
    QTimer darkenTimer;
    QElapsedTimer elapsedTimer;
    int samplesPerMillisecond{0};
    double millisecondsPerSample{0.0};
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
    int beamAlpha;
    double beamWidth;
    double beamIntensity;

    void render();
    void calcBeamAlpha();
signals:

};

#endif // SCOPEWIDGET_H
