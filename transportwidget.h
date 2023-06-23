/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#ifndef TRANSPORTWIDGET_H
#define TRANSPORTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLCDNumber>

class TransportWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TransportWidget(QWidget *parent = nullptr);

	bool getPaused() const;
	void setPaused(bool value);
	void setLength(int milliseconds);
	void setButtonsEnabled(bool value);

signals:
	void returnToStartClicked();
	void playPauseToggled(bool paused);
	void positionChangeRequested(int milliseconds);

public slots:
	void setPosition(int milliseconds);

protected:
	int heightForWidth(int) const;

private:
	QSlider* slider;
	QPushButton* rtsButton;
	QPushButton* playPauseButton;
	QLCDNumber* hh;
	QLCDNumber* mm;
	QLCDNumber* ss;
	QLCDNumber* ms;

	bool paused{true};
};

#endif // TRANSPORTWIDGET_H
