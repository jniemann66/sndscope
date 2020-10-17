#include "mainwindow.h"

#include <scopewidget.h>
#include <QDebug>
#include <QDockWidget>

#include "transportwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto scopeWidget = new ScopeWidget(this);
    auto transportDock = new QDockWidget("Transport", this);
    auto transportWidget = new TransportWidget(transportDock);

    setCentralWidget(scopeWidget);
    transportDock->setAllowedAreas(Qt::RightDockWidgetArea);
    transportDock->setWidget(transportWidget);
    connect(scopeWidget, &ScopeWidget::renderedFrame, transportWidget, &TransportWidget::setPosition);

        // qDebug() << scopeWidget->loadSoundFile("/home/judd/IQ/WFM/12-03-39_105500kHz.wav");
        // qDebug() << scopeWidget->loadSoundFile("/home/judd/IQ/WFM/21-20-26_90700kHz.wav");

    connect(transportWidget, &TransportWidget::playPauseToggled, scopeWidget, &ScopeWidget::setPaused);
    connect(transportWidget, &TransportWidget::returnToStartClicked, scopeWidget, &ScopeWidget::returnToStart);

    auto loadResult = scopeWidget->loadSoundFile("/home/judd/Music/05 Spirals.wav");
    if (loadResult.first) {
        transportWidget->setLength(scopeWidget->getLengthMilliseconds());
    }
}

MainWindow::~MainWindow()
{
}

