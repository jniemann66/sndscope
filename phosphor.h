#ifndef PHOSPHOR_H
#define PHOSPHOR_H

#include <QColor>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct Layer
{
    QColor color;
    double persistence;
};

struct Phosphor
{
public:
    QString name;
    QVector<Layer> layers;

    void fromJson(const QJsonObject& o);
    QJsonObject toJson() const;

};

#endif // PHOSPHOR_H
