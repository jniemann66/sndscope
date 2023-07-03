/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#include "mainwindow.h"

//#include "interpolator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

//	Interpolator<double, double, 2> i2;

//	QVector<double> input{0,1,0,-1, 0,1,0,-1, 0,1,0,-1, 0,1,0,-1, 0,1,0,-1, 0,1,0,-1, 0,1,0,-1, 0,1,0,-1};
//	QVector<double> output(64, 0.0);
//	i2.upsample(output.data(), input.constData(), input.size());
//	qDebug() << output;
//	qDebug() << "Done";

	return a.exec();
}
