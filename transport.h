#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLCDNumber>

class Transport : public QWidget
{
    Q_OBJECT
public:
    explicit Transport(QWidget *parent = nullptr);

    bool getPaused() const;
    void setPaused(bool value);
    void setLength(int milliseconds);

signals:
    void returnToStartClicked();
    void playPauseToggled(bool paused);

public slots:
    void setPosition(int milliseconds);

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

#endif // TRANSPORT_H
