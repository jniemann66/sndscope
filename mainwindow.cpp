/*
* Copyright (C) 2020 - 2021 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "mainwindow.h"

#include <scopewidget.h>
#include <QDebug>
#include <QDockWidget>
#include <QDropEvent>
#include <QMimeData>
#include <QDir>
#include <QMenuBar>
#include <QFileDialog>

#include "transportwidget.h"
#include "displaysettingswidget.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    auto scopeWidget = new ScopeWidget(this);
    auto transportDock = new QDockWidget("Transport", this);
    auto transportWidget = new TransportWidget(transportDock);
    auto displaySettingsDock = new QDockWidget("Display", this);
    auto displaySettingsWidget = new DisplaySettingsWidget(displaySettingsDock);

    setCentralWidget(scopeWidget);
    transportDock->setWidget(transportWidget);
    transportDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, transportDock);

    displaySettingsDock->setWidget(displaySettingsWidget);
    displaySettingsDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, displaySettingsDock);

    displaySettingsWidget->setBrightness(scopeWidget->getBrightness());
    displaySettingsWidget->setFocus(scopeWidget->getFocus());
    displaySettingsWidget->setPersistence(scopeWidget->getPersistence());

    setWindowTitle("Drag & drop a wave file");
    setAcceptDrops(true);

    fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Open ...", [this, scopeWidget, transportWidget]{
       QString path = QFileDialog::getOpenFileName(this,
                       tr("Open Sound File"), QDir::homePath(), tr("Sound Files (*.aif *.aifc *.aiff *.au *.avr *.caf *.flac *.htk *.iff *.mat *.mpc *.oga *.paf *.pvf *.raw *.rf64 *.sd2 *.sds *.sf *.voc *.w64 *.wav *.wve *.xi)"));
       auto loadResult = scopeWidget->loadSoundFile(path);
       transportWidget->setButtonsEnabled(loadResult.first);
       if (loadResult.first) {
           transportWidget->setLength(scopeWidget->getLengthMilliseconds());
           setWindowTitle(path);
       }
    });

    connect(scopeWidget, &ScopeWidget::renderedFrame, transportWidget, &TransportWidget::setPosition);
    connect(transportWidget, &TransportWidget::playPauseToggled, scopeWidget, &ScopeWidget::setPaused);
    connect(transportWidget, &TransportWidget::returnToStartClicked, scopeWidget, &ScopeWidget::returnToStart);
    connect(transportWidget, &TransportWidget::positionChangeRequested, scopeWidget, &ScopeWidget::gotoPosition);
    connect(this, &MainWindow::fileDrop, [this, scopeWidget, transportWidget](const QString& path){
        auto loadResult = scopeWidget->loadSoundFile(path);
        transportWidget->setButtonsEnabled(loadResult.first);
        if (loadResult.first) {
            transportWidget->setLength(scopeWidget->getLengthMilliseconds());
            setWindowTitle(path);
        }
    });

    connect(displaySettingsWidget, &DisplaySettingsWidget::brightnessChanged, this, [scopeWidget](double value){
        scopeWidget->setBrightness(value);
    });

    connect(displaySettingsWidget, &DisplaySettingsWidget::focusChanged, this, [scopeWidget](double value){
        scopeWidget->setFocus(value);
    });

    connect(displaySettingsWidget, &DisplaySettingsWidget::persistenceChanged, this, [scopeWidget](int value){
       scopeWidget->setPersistence(value);
    });

    connect(displaySettingsWidget, &DisplaySettingsWidget::phosphorColorChanged, this, [scopeWidget](QVector<QColor> colors){
       if(colors.count() > 0) {
            scopeWidget->setPhosphorColor(colors.at(0).rgba());
       }
    });

    connect(displaySettingsWidget, &DisplaySettingsWidget::multiColorPhosphorChanged, this, [scopeWidget](bool multi, const QColor& altColor){
       scopeWidget->setMultiColor(multi, altColor);
    });

    connect(displaySettingsWidget, &DisplaySettingsWidget::wipeScreenRequested, scopeWidget, &ScopeWidget::wipeScreen);

    // get a list of audio output devices
    auto devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput);
    qDebug() << "Audio Output Devices";
    for(const QAudioDeviceInfo& device : devices) {
        qDebug() << device.deviceName();
    }

}

MainWindow::~MainWindow()
{
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    auto mimeData = event->mimeData();
	if(mimeData->hasText()) {
        QUrl url{mimeData->text()};
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    auto mimeData = event->mimeData();
	if(mimeData->hasText()) {
        QUrl url{mimeData->text()};
		QString path = QDir::toNativeSeparators(url.path());

#ifdef Q_OS_WIN
		if(path.startsWith('\\')) {
			path.remove(0, 1);
		}
#endif

		emit fileDrop(path);
    }
}
