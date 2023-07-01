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
	ScopeDisplay(QWidget* parent = nullptr) : QWidget(parent), pixmap(800, 640)
    {
        setAutoFillBackground(false);
        resizeCooldownTimer.setSingleShot(true);
        resizeCooldownTimer.setInterval(50);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        // actual resizing of pixmap is only done after waiting for resize events to settle-down
		connect(&resizeCooldownTimer, &QTimer::timeout, this, [this]{
            if (pixmap.height() != height()) {
                const int h = height();
				const int w = aspectRatio.first * h / aspectRatio.second;
                qDebug().noquote() << QStringLiteral("adjusting pixmap resolution to %1x%2").arg(w).arg(h);
                pixmap = pixmap.scaled(w, h);
				calcGraticule();
                emit pixmapResolutionChanged(pixmap.size());
            }
        });
	}

	// getters
    QPixmap* getPixmap()
    {
        return &pixmap;
    }

    bool getAllowPixmapResolutionChange() const
    {
        return allowPixmapResolutionChange;
    }

	QPair<int, int> getAspectRatio() const
	{
		return aspectRatio;
	}

	QPair<int, int> getDivisions() const
	{
		return divisions;
	}

	// setters

    void setAllowPixmapResolutionChange(bool value)
    {
        allowPixmapResolutionChange = value;
    }


	void setAspectRatio(const QPair<int, int> &newAspectRatio)
	{
		aspectRatio = newAspectRatio;
	}

	void setDivisions(const QPair<int, int> &newDivisions)
	{
		divisions = newDivisions;
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

		p.setRenderHint(QPainter::Antialiasing, true);
		QPen graticulePen(graticuleColor, 1.5);
		p.setPen(graticulePen);
		p.drawLines(graticuleLines);
    }

    void resizeEvent(QResizeEvent *event) override
    {
		const int h = event->size().height();
		const int w = aspectRatio.first * h / aspectRatio.second;
		setFixedWidth(w);

		setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		updateGeometry();

		if(allowPixmapResolutionChange) {
			resizeCooldownTimer.start();
		}

		calcGraticule();
	}

	void calcGraticule()
	{
		const double w = width() - 0.5;
		const double h = height() - 0.5;
		const double cx = 0.5 * w;
		const double cy = 0.5 * h;

		const double divx = w / divisions.first;
		const double divy = h / divisions.second;

		graticuleLines.clear();
		double gx = 0.5;
		double gy = 0.5;

		constexpr double tl = 4.0;
		const double txd = 0.2 * divx;
		const double tyd = 0.2 * divy;

		const double ty0 = cy - tl;
		const double ty1 = cy + tl;

		const double tx0 = cx - tl;
		const double tx1 = cx + tl;

		for(int i = 0; i <= divisions.first; i++) {
			graticuleLines.append(QPointF{gx, 0});
			graticuleLines.append(QPointF{gx, h});
			double tx = gx + txd;
			for(int j = 1; j <= 4; j++) {
				graticuleLines.append(QPointF{tx, ty0});
				graticuleLines.append(QPointF{tx, ty1});
				tx += txd;
			}
			gx += divx;
		}

		for(int i = 0; i <= divisions.second; i++) {
			graticuleLines.append(QPointF{0, gy});
			graticuleLines.append(QPointF{w, gy});
			double ty = gy + tyd;
			for(int j = 1; j <= 4; j++) {
				graticuleLines.append(QPointF{tx0, ty});
				graticuleLines.append(QPointF{tx1, ty});
				ty += tyd;
			}
			gy += divy;
		}
	}

private:
	QColor graticuleColor{112, 112, 112, 160};
	QPair<int, int> divisions{10, 8};
	QPair<int, int> aspectRatio{5, 4};
	QTimer resizeCooldownTimer;
    QPixmap pixmap;
	bool allowPixmapResolutionChange{true};
	QVector<QPointF> graticuleLines;

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
	void setOutputDevice(const QAudioDeviceInfo &newOutputDeviceInfo);
	void setShowTrigger(bool val);

	void plotTest();
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

	QVector<QPointF> testPlot;
	QVector<QPointF> plotBuffer;

	PlotMode plotMode{XY};
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
	double w;
	double h;
	double divx;
	double divy;

	int darkenAlpha;
	int beamAlpha;
	double beamWidth;
	double beamIntensity;

	void calcScaling();
	void render();
	void renderTest();
	void calcBeamAlpha();

	void renderTrigger(QPainter *painter);
	void makeTestPlot();
signals:

};



#endif // SCOPEWIDGET_H
