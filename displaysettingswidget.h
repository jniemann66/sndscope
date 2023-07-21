/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#ifndef DISPLAYSETTINGSWIDGET_H
#define DISPLAYSETTINGSWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QDial>
#include <QComboBox>
#include <QPushButton>

#include "phosphor.h"

class DisplaySettingsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit DisplaySettingsWidget(QWidget *parent = nullptr);
	QSize sizeHint() const override;

	// getters
	double getBrightness() const;
	double getFocus() const;
	int getPersistence() const;

	// setters
	void setBrightness(double value);
	void setFocus(double value);
	void setPersistence(int value);

signals:
	void brightnessChanged(double brightness);
	void focusChanged(double focus);
	void phosphorColorChanged(QVector<QColor>);
	void persistenceChanged(int persistence);
	void wipeScreenRequested();

protected:

private:
	QDial* brightnessControl{nullptr};
	QDial* focusControl{nullptr};
	QComboBox* phosphorSelectControl{nullptr};
	QDial* persistenceControl{nullptr};
	QMap<QString, Phosphor> phosphors;
	QPushButton* clearScreenButton{nullptr};

	QPair<bool, QString> loadPhosphors(const QString &filename);
};

#endif // DISPLAYSETTINGSWIDGET_H
