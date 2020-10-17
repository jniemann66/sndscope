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
