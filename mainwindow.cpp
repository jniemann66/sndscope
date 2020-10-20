#include "mainwindow.h"

#include <scopewidget.h>
#include <QDebug>
#include <QDockWidget>
#include <QDropEvent>
#include <QMimeData>

#include "transportwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto scopeWidget = new ScopeWidget(this);
    auto transportDock = new QDockWidget("Transport", this);
    auto transportWidget = new TransportWidget(transportDock);

    setCentralWidget(scopeWidget);
    transportDock->setWidget(transportWidget);
    transportDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, transportDock);
    setWindowTitle("Drag & drop a wave file");
    setAcceptDrops(true);

    connect(scopeWidget, &ScopeWidget::renderedFrame, transportWidget, &TransportWidget::setPosition);
    connect(transportWidget, &TransportWidget::playPauseToggled, scopeWidget, &ScopeWidget::setPaused);
    connect(transportWidget, &TransportWidget::returnToStartClicked, scopeWidget, &ScopeWidget::returnToStart);
    connect(transportWidget, &TransportWidget::positionChangeRequested, scopeWidget, &ScopeWidget::gotoPosition);
    connect(this, &MainWindow::fileDrop, [this, scopeWidget, transportWidget](const QString& path){
        auto loadResult = scopeWidget->loadSoundFile(path);
        if (loadResult.first) {
            transportWidget->setLength(scopeWidget->getLengthMilliseconds());
            setWindowTitle(path);
        }
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    auto mimeData = event->mimeData();
    if(mimeData->hasFormat("text/plain")) {
        QUrl url{mimeData->text()};
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    auto mimeData = event->mimeData();
    if(mimeData->hasFormat("text/plain")) {
        QUrl url{mimeData->text()};
        emit fileDrop(url.path());
    }
}
