#include "mainwindow.h"

#include <scopewidget.h>
#include <QDebug>
#include <QDockWidget>

#include "transport.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto scopeWidget = new ScopeWidget(this);
    // qDebug() << scopeWidget->loadSoundFile("/home/judd/IQ/WFM/12-03-39_105500kHz.wav");
    // qDebug() << scopeWidget->loadSoundFile("/home/judd/IQ/WFM/21-20-26_90700kHz.wav");
    qDebug() << scopeWidget->loadSoundFile("/home/judd/Music/05 Spirals.wav");
    setCentralWidget(scopeWidget);

    auto transportDock = new QDockWidget("Transport", this);
    transportDock->setAllowedAreas(Qt::RightDockWidgetArea);
    auto transport = new Transport(transportDock);
    transportDock->setWidget(transport);
    connect(scopeWidget, &ScopeWidget::renderedFrame, transport, &Transport::setPosition);

}

MainWindow::~MainWindow()
{
}

