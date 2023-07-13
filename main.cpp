/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#include <QDebug>

#include <QColor>

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();


	QColor r{127, 0, 0, 128};
	qDebug() << r.toRgb();
	r.setAlphaF(0.5 + 0.00390625);
	qDebug() << r.toRgb();

	return a.exec();
}
