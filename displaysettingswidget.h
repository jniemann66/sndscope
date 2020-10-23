#ifndef DISPLAYSETTINGSWIDGET_H
#define DISPLAYSETTINGSWIDGET_H

#include <QWidget>

#include <QSlider>

class DisplaySettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DisplaySettingsWidget(QWidget *parent = nullptr);
    QSize sizeHint() const override;

    double getBrightness() const;
    void setBrightness(double value);

    double getFocus() const;
    void setFocus(double value);

signals:
    void brightnessChanged(double brightness);
    void focusChanged(double focus);

protected:


private:
    QSlider* brightnessSlider{nullptr};
    QSlider* focusSlider{nullptr};
};

#endif // DISPLAYSETTINGSWIDGET_H
