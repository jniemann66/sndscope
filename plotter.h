/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/


#ifndef PLOTTER_H
#define PLOTTER_H

#include "sweepparameters.h"
#include "plotmode.h"

#include <QObject>
#include <QPainter>
#include <QImage>

#ifdef SNDSCOPE_BLEND2D
	#include <blimagewrapper.h>
#endif

class Plotter : public QObject
{
	Q_OBJECT
public:
	explicit Plotter(QObject *parent = nullptr);
	void render(const QVector<QVector<float> > &inputBuffers, int64_t framesAvailable, int64_t currentFrame);
	void calcScaling();

	// getters
	SweepParameters getSweepParameters() const;
	double getTimeLimit_ms() const;
	bool getFreshRender() const;
	QPixmap *getPixmap() const;
	int getAudioFramesPerMs() const;
	QColor getDarkencolor() const;
	int getDarkenNthFrame() const;
	int64_t getExpectedFrames() const;
	qreal getBeamWidth() const;
	qreal getBeamIntensity() const;
	int getNumInputChannels() const;
	QColor getPhosphorColor() const;
	QPainter::CompositionMode getCompositionMode() const;
	Plotmode getPlotMode() const;
	bool getconnectSamples() const;
	bool getShowTrigger() const;

	// setters
	void setSweepParameters(const SweepParameters &newSweepParameters);
	void setTimeLimit_ms(double newTimeLimit_ms);
	void setFreshRender(bool newFreshRender);
	void setPixmap(QPixmap *newPixmap);
	void setAudioFramesPerMs(int newAudioFramesPerMs);
	void setDarkencolor(const QColor &newDarkencolor);
	void setDarkenNthFrame(int newDarkenNthFrame);
	void setExpectedFrames(int64_t newExpectedFrames);
	void setBeamWidth(qreal newBeamWidth);
	void setBeamIntensity(qreal newBeamIntensity);
	void setNumInputChannels(int newNumInputChannels);
	void setPhosphorColor(const QColor &newPhosphorColor);
	void setCompositionMode(QPainter::CompositionMode newCompositionMode);
	void setPlotMode(Plotmode newPlotMode);
	void setconnectSamples(bool newconnectSamples);
	void setShowTrigger(bool newShowTrigger);

	void drawTrigger(QPainter *painter);

signals:
	void renderedFrame(int64_t frame);

private:
	QVector<QPointF> plotBuffer;
	SweepParameters sweepParameters;
	QPixmap* pixmap{nullptr};
	double timeLimit_ms;
	int64_t expectedFrames{0ll}; // number of audioframes expected per plotTimer timeout
	bool freshRender{false};
	int audioFramesPerMs{0};
	Plotmode plotMode;
	bool connectSamples{false};
	qreal cx;
	qreal cy;
	qreal w;
	qreal h;
	int darkenCooldownCounter{1};
	QPainter::CompositionMode compositionMode{QPainter::CompositionMode_SourceOver};
	qreal beamWidth;
	qreal beamIntensity;
	QColor phosphorColor{0x3e, 0xff, 0x6f, 0xff};
	QColor darkencolor{0, 0, 0, 0};
	int darkenNthFrame{1};
	int numInputChannels;
	bool showTrigger{false};

	//bl.createFromData(img->width(), img->height(), BL_FORMAT_PRGB32, img->bits(), img->bytesPerLine());
#ifdef SNDSCOPE_BLEND2D
	BLImageWrapper *blImageWrapper;
public:
	BLImageWrapper *getBlImageWrapper() const;
	void setBlImageWrapper(BLImageWrapper *newBlImageWrapper);
#endif

};

#endif // PLOTTER_H
