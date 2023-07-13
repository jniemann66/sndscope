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
#include <QThread>

#include <sndfile.hh>

#include "plotmode.h"
#include "sweepparameters.h"
#include "audiocontroller.h"
#include "upsampler.h"
#include "renderer.h"

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

	bool getShowGraticule() const
	{
		return showGraticule;
	}
	void setShowGraticule(bool newShowGraticule)
	{
		showGraticule = newShowGraticule;
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

		if(showGraticule) {
			p.setRenderHint(QPainter::Antialiasing, true);
			QPen graticulePen(graticuleColor, 1.5);
			p.setPen(graticulePen);
			p.drawLines(graticuleLines);
		}
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
	bool showGraticule{true};
};

// ScopeWidget : the heart of the Oscilloscope

class ScopeWidget : public QWidget
{
	Q_OBJECT
	friend class Renderer;
	static constexpr int upsampleFactor = 4;

	QThread renderThread;

public:
	explicit ScopeWidget(QWidget *parent = nullptr);
	~ScopeWidget();

	QPair<bool, QString> loadSoundFile(const QString &filename);

	// getters
	Plotmode getPlotmode() const;
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
	void setPaused(bool value);
	void setBrightness(double value);
	void setFocus(double value);
	void setPersistence(double time_ms);
	void setPhosporColors(const QVector<QColor> &colors);
	void setBackgroundColor(const QColor &value);
	void setOutputDevice(const QAudioDeviceInfo &newOutputDeviceInfo);
	void setShowTrigger(bool val);
	void plotTest();

	bool getUpsampling() const;

public slots:
	void returnToStart();
	void gotoPosition(int64_t milliSeconds);
	void wipeScreen();
	void setAudioVolume(qreal linearVolume);
	void setSweepParameters(const SweepParameters &newSweepParameters);
	void setPlotmode(Plotmode newPlotmode);
	void setUpsampling(bool val);

signals:
	void loadedFile();
	void renderedFrame(int positionMilliseconds);
	void outputVolume(qreal linearVol);

protected:

private:
    ScopeDisplay* scopeDisplay{nullptr};
	AudioController *audioController{nullptr};
	Renderer *renderer{nullptr};
	QIODevice* pushOut{nullptr};
	QHBoxLayout *screenLayout{nullptr};
	std::unique_ptr<SndfileHandle> sndfile;
	QAudioFormat audioFormat;
	QAudioDeviceInfo outputDeviceInfo;
	UpSampler<float, float, upsampleFactor> upsampler;

	// audio buffers
	QVector<float> rawinputBuffer; // interleaved
	QVector<QVector<float>> inputBuffers; // de-interleaved

	// timing
	QTimer plotTimer;
    QTimer screenUpdateTimer;
	QElapsedTimer elapsedTimer;

	Plotmode plotMode{XY};
	bool showTrigger{false};
	SweepParameters sweepParameters;

	bool upsampling{false};
	int numInputChannels{0};
	int audioFramesPerMs{0};
	double msPerAudioFrame{0.0};

	// audio frame accounting
	int64_t startFrame{0ll}; // start position for playback
	int64_t currentFrame{0ll}; // start position of next read
	int64_t framesRead{0ll}; // number of audioframes last read from file
	int64_t framesAvailable{011}; // number of audioframes currently stored in input buffers; after upsampling
	int64_t expectedFrames{0ll}; // number of audioframes expected per plotTimer timeout
	int64_t maxFramesToRead{0ll}; // limit of how many audioframes can fit in buffer
	int64_t totalFrames{0ll}; // total number of audioframes in sound file

	bool fileLoaded{false};
	bool paused{true};

	// crt properties
	qreal brightness{80.0};
	qreal focus{80.0};
	qreal persistence{32.0};

	QColor backgroundColor{0, 0, 0, 255};
	int beamAlpha; // todo: try AlphaF (qreal)

	// plot dimensions
	qreal cx;
	qreal cy;
	qreal w;
	qreal h;

	// private functions
	void readInput();
	void calcBeamAlpha();
	void drawTrigger(QPainter *painter);
	void makeTestPlot();
};

#endif // SCOPEWIDGET_H
