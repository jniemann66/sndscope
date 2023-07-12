#ifndef RENDERER_H
#define RENDERER_H

#include "sweepparameters.h"
#include "plotmode.h"

#include <QObject>
#include <QPainter>

class Renderer : public QObject
{
	Q_OBJECT
public:
	explicit Renderer(QObject *parent = nullptr);
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

	void drawTrigger(QPainter *painter);
	bool getShowTrigger() const;
	void setShowTrigger(bool newShowTrigger);

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
};

#endif // RENDERER_H
