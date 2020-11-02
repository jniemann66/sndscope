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
