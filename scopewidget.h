/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
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
#include <QHBoxLayout>
#include <QAudioDeviceInfo>

#include <sndfile.hh>

#include "sweepparameters.h"
#include "audiocontroller.h"

enum PlotMode
{
	XY,
	MidSide,
	Sweep
};

// ScopeDisplay : this is the Oscilloscope's screen
// it owns a QPixmap as an image buffer, which is accessed via getPixmap()

class ScopeDisplay : public QWidget
{
	Q_OBJECT
public:
    ScopeDisplay(QWidget* parent = nullptr) : QWidget(parent), pixmap(640, 640)
    {
        setAutoFillBackground(false);
        resizeCooldownTimer.setSingleShot(true);
        resizeCooldownTimer.setInterval(50);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        // actual resizing of pixmap is only done after waiting for resize events to settle-down
		connect(&resizeCooldownTimer, &QTimer::timeout, this, [this]{
            if (pixmap.height() != height()) {
                const int h = height();
                const int w = constrainToSquare ? h : width();
                qDebug().noquote() << QStringLiteral("adjusting pixmap resolution to %1x%2").arg(w).arg(h);
                pixmap = pixmap.scaled(w, h);
                emit pixmapResolutionChanged(pixmap.size());
            }
        });
	}

	// getters
    QPixmap* getPixmap()
    {
        return &pixmap;
    }

    bool getConstrainToSquare() const
    {
        return constrainToSquare;
    }

    bool getAllowPixmapResolutionChange() const
    {
        return allowPixmapResolutionChange;
    }

	// setters
    void setConstrainToSquare(bool value)
    {
        constrainToSquare = value;
    }

    void setAllowPixmapResolutionChange(bool value)
    {
        allowPixmapResolutionChange = value;
    }

signals:
	void pixmapResolutionChanged(const QSizeF& size);

protected:
    QSize sizeHint() const override
    {
        return pixmap.size();
    }

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
		QPainter p(this);
		p.setRenderHint(QPainter::TextAntialiasing, false);

        if(size() == pixmap.size()) {
            p.drawPixmap(0, 0, pixmap);
        } else {
            p.drawPixmap(0, 0, pixmap.scaled(size()));
        }
    }

    void resizeEvent(QResizeEvent *event) override
    {
		if(constrainToSquare) {
			auto h = event->size().height();
            setFixedWidth(h);
        }

        if(constrainToSquare) {
            setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        } else {
            setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        }
        updateGeometry();


        if(allowPixmapResolutionChange) {
			resizeCooldownTimer.start();
		}
	}

private:
	QTimer resizeCooldownTimer;
    QPixmap pixmap;
    bool constrainToSquare{false};
	bool allowPixmapResolutionChange{true};
};

// ScopeWidget : the heart of the Oscilloscope

class ScopeWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ScopeWidget(QWidget *parent = nullptr);
	QPair<bool, QString> loadSoundFile(const QString &filename);

	// getters
	PlotMode getChannelMode() const;
	int getLengthMilliseconds() const;
	bool getPaused() const;
	int64_t getTotalFrames() const;
	double getBrightness() const;
	double getFocus() const;
	QColor getPhosphorColor() const;
	double getPersistence() const;
	bool getMultiColor() const;
	QColor getBackgroundColor() const;
	bool getConstrainToSquare() const;
	SweepParameters getSweepParameters() const;
	QAudioDeviceInfo getOutputDeviceInfo() const;
	bool getShowTrigger() const;

	// setters
	void setChannelMode(PlotMode newChannelMode);
	void setPaused(bool value);
	void setTotalFrames(const int64_t &value);
	void setBrightness(double value);
	void setFocus(double value);
	void setPersistence(double time_ms);
	void setPhosporColors(const QVector<QColor> &colors);
	void setBackgroundColor(const QColor &value);
	void setConstrainToSquare(bool value);
	void setOutputDevice(const QAudioDeviceInfo &newOutputDeviceInfo);
	void setShowTrigger(bool val);

public slots:
	void returnToStart();
	void gotoPosition(int64_t milliSeconds);
	void wipeScreen();
	void setAudioVolume(qreal linearVolume);
	void setSweepParameters(const SweepParameters &newSweepParameters);

signals:
	void loadedFile();
	void renderedFrame(int positionMilliseconds);
	void outputVolume(qreal linearVol);

protected:

  //  void resizeEvent(QResizeEvent *event) {

   // }
private:
    ScopeDisplay* scopeDisplay{nullptr};
	AudioController *audioController{nullptr};
	QIODevice* pushOut{nullptr};
	QHBoxLayout *screenLayout{nullptr};

	std::unique_ptr<SndfileHandle> sndfile;
	QVector<float> inputBuffer;
	QAudioFormat audioFormat;
	QAudioDeviceInfo outputDeviceInfo;
	QTimer plotTimer;
    QTimer screenUpdateTimer;
	QElapsedTimer elapsedTimer;
	QVector<QPointF> plotBuffer;

	PlotMode plotMode{Sweep};
	bool showTrigger{false};
	SweepParameters sweepParameters;
	int inputChannels{0};
	int audioFramesPerMs{0};
	double msPerAudioFrame{0.0};
	int64_t startFrame{0};
	int64_t currentFrame{0ll};
	int64_t maxFramesToRead{0};
	int64_t totalFrames{0ll};
	bool fileLoaded{false};
	bool paused{true};
    bool constrainToSquare{false};
	double brightness{80.0};
	double focus{80.0};
	double persistence{32.0};
	int darkenNthFrame{1};
	int darkenCooldownCounter{1};
	QColor phosphorColor{0x3e, 0xff, 0x6f, 0xff};
	QColor darkencolor{0, 0, 0, 0};
	QColor backgroundColor{0, 0, 0, 255};
	QPainter::CompositionMode compositionMode{QPainter::CompositionMode_SourceOver};
    bool freshRender{false};

	// midpoint of pixmap
	double cx;
	double cy;
	int darkenAlpha;
	int beamAlpha;
	double beamWidth;
	double beamIntensity;

	void calcScaling();
	void render();
	void calcBeamAlpha();

	void renderTrigger(QPainter *painter);
signals:

};



#endif // SCOPEWIDGET_H
