#include "phosphor.h"

void Phosphor::fromJson(const QJsonObject &o)
{
    name = o.value("name").toString();
    if(o.value("layers").isArray()) {
        QJsonArray a = o.value("layers").toArray();
        for(int i = 0; i < a.count(); i++) {
            PhosphorLayer layer;
            if(a.at(i).isObject()) {
                QJsonObject layerObj = a.at(i).toObject();
                layer.color.setRed(layerObj.value("red").toInt(94));
                layer.color.setGreen(layerObj.value("green").toInt(255));
                layer.color.setBlue(layerObj.value("blue").toInt(0));
                layer.persistence = layerObj.value("persistence").toInt(20);
            } else { // set defaults
                layer.color.setRgb(94, 255, 0);
                layer.persistence = 20;
            }
            layers.append(layer);
        }
    }
}

QJsonObject Phosphor::toJson() const
{
    QJsonObject o;
    o.insert("name", name);
    QJsonArray a;
    for(const PhosphorLayer& layer : layers) {
        QJsonObject layerObj;
        layerObj.insert("red", layer.color.red());
        layerObj.insert("green", layer.color.green());
        layerObj.insert("blue", layer.color.blue());
        layerObj.insert("persistence", layer.persistence);
        a.append(layerObj);
    }
    o.insert("layers", a);
    return o;
}
