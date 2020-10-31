#ifndef DISPLAYSETTINGSWIDGET_H
#define DISPLAYSETTINGSWIDGET_H

#include <QWidget>

#include <QSlider>
#include <QDial>

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

    int getPersistence() const;
    void setPersistence(int value);

signals:
    void brightnessChanged(double brightness);
    void focusChanged(double focus);
    void persistenceChanged(int persistence);

protected:


private:
    QDial* brightnessControl{nullptr};
    QDial* focusControl{nullptr};
    QDial* persistenceControl{nullptr};
};

#endif // DISPLAYSETTINGSWIDGET_H
