#include "mainwindow.h"

#include <scopewidget.h>
#include <QDebug>
#include <QDockWidget>

#include "transport.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto scopeWidget = new ScopeWidget(this);
    auto transportDock = new QDockWidget("Transport", this);
    auto transport = new Transport(transportDock);

    setCentralWidget(scopeWidget);
    transportDock->setAllowedAreas(Qt::RightDockWidgetArea);
    transportDock->setWidget(transport);
    connect(scopeWidget, &ScopeWidget::renderedFrame, transport, &Transport::setPosition);

        // qDebug() << scopeWidget->loadSoundFile("/home/judd/IQ/WFM/12-03-39_105500kHz.wav");
        // qDebug() << scopeWidget->loadSoundFile("/home/judd/IQ/WFM/21-20-26_90700kHz.wav");

    connect(transport, &Transport::playPauseToggled, scopeWidget, &ScopeWidget::setPaused);

    auto loadResult = scopeWidget->loadSoundFile("/home/judd/Music/05 Spirals.wav");
    if (loadResult.first) {
        transport->setLength(scopeWidget->getLengthMilliseconds());
    }
}

MainWindow::~MainWindow()
{
}

