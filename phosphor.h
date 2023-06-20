/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef PHOSPHOR_H
#define PHOSPHOR_H

#include <QColor>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct PhosphorLayer
{
	QColor color;
	double persistence;
};

struct Phosphor
{
public:
	QString name;
	QVector<PhosphorLayer> layers;

	void fromJson(const QJsonObject& o);
	QJsonObject toJson() const;

};

#endif // PHOSPHOR_H
