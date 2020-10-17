#ifndef SCOPEWIDGET_H
#define SCOPEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QTime>

#include <sndfile.hh>

class SizeTracker : public QObject
{
    Q_OBJECT
public:

    SizeTracker(QObject* parent);
    double cx;
    double cy;
    double w;
    double h;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

class ScopeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScopeWidget(QWidget *parent = nullptr);
    QPair<bool, QString> loadSoundFile(const QString &filename);
    int getLengthMilliseconds();

    bool getPaused() const;
    void setPaused(bool value);

signals:
    void renderedFrame(int positionMilliseconds);

protected:
    int heightForWidth(int) const override;

private:
    QLabel* screenWidget;
    std::unique_ptr<SndfileHandle> h;
    QVector<double> inputBuffer;
    SizeTracker* sizeTracker;

    QPixmap pixmap;
    QTimer darkenTimer;
    QElapsedTimer elapsedTimer;
    int samplesPerMillisecond{0};
    double millisecondsPerSample{0.0};
    int64_t frame{0ll};
    bool paused{true};

    void plot();
signals:

};

#endif // SCOPEWIDGET_H
