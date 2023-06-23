/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
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

#include "audiosettingswidget.h"
#include "transportwidget.h"
#include "displaysettingswidget.h"

MainWindow::MainWindow(QWidget	*parent) : QMainWindow(parent)
{
	auto scopeWidget = new ScopeWidget(this);
	auto transportDock = new QDockWidget("Transport", this);
	auto transportWidget = new TransportWidget(transportDock);
	auto displaySettingsDock = new QDockWidget("Display", this);
	auto displaySettingsWidget = new DisplaySettingsWidget(displaySettingsDock);
	auto audioSettingsDock = new QDockWidget("Audio Settings", this);
	auto audioSettingsWidget = new AudioSettingsWidget(audioSettingsDock);

	setCentralWidget(scopeWidget);
	transportDock->setWidget(transportWidget);
	transportDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::BottomDockWidgetArea, transportDock);

	displaySettingsDock->setWidget(displaySettingsWidget);
	displaySettingsDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::RightDockWidgetArea, displaySettingsDock);

	audioSettingsDock->setWidget(audioSettingsWidget);
	audioSettingsDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::RightDockWidgetArea, audioSettingsDock);

	displaySettingsWidget->setBrightness(scopeWidget->getBrightness());
	displaySettingsWidget->setFocus(scopeWidget->getFocus());
	displaySettingsWidget->setPersistence(scopeWidget->getPersistence());



    // todo: read from settings file
    bool squareDisplayOnly = true;
    scopeWidget->setConstrainToSquare(squareDisplayOnly);
    //

	setWindowTitle("Drag & drop a wave file");
	setAcceptDrops(true);

	fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction("&Open ...", this, [this, scopeWidget, transportWidget]{
		QString path = QFileDialog::getOpenFileName(this,
													tr("Open Sound File"),
													QDir::homePath(),
													tr("Sound Files (*.aif *.aifc *.aiff *.au *.avr *.caf *.flac *.htk *.iff *.mat *.mpc *.oga *.paf *.pvf *.raw *.rf64 *.sd2 *.sds *.sf *.voc *.w64 *.wav *.wve *.xi)")
													);
		auto loadResult = scopeWidget->loadSoundFile(path);
		transportWidget->setButtonsEnabled(loadResult.first);
		if (loadResult.first) {
			transportWidget->setLength(scopeWidget->getLengthMilliseconds());
			setWindowTitle(path);
		}
	});

	preferencesMenu = menuBar()->addMenu("&Preferences");
	QAction* squareDisplayOption = preferencesMenu->addAction("Square Display");
    squareDisplayOption->setCheckable(true);
    squareDisplayOption->setChecked(squareDisplayOnly);

    connect(squareDisplayOption, &QAction::triggered, this, [scopeWidget](bool checked){
        scopeWidget->setConstrainToSquare(checked);
	});

	connect(scopeWidget, &ScopeWidget::renderedFrame, transportWidget, &TransportWidget::setPosition);
	connect(transportWidget, &TransportWidget::playPauseToggled, scopeWidget, &ScopeWidget::setPaused);
	connect(transportWidget, &TransportWidget::returnToStartClicked, scopeWidget, &ScopeWidget::returnToStart);
	connect(transportWidget, &TransportWidget::positionChangeRequested, scopeWidget, &ScopeWidget::gotoPosition);
	connect(this, &MainWindow::fileDrop, this, [this, scopeWidget, transportWidget](const QString& path){
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

	connect(displaySettingsWidget, &DisplaySettingsWidget::phosphorColorChanged, this, [scopeWidget](const QVector<QColor>& colors){
		scopeWidget->setPhosporColors(colors);
	});

	connect(displaySettingsWidget, &DisplaySettingsWidget::persistenceChanged, this, [scopeWidget](int value){
		scopeWidget->setPersistence(value);
	});

	connect(displaySettingsWidget, &DisplaySettingsWidget::wipeScreenRequested, scopeWidget, &ScopeWidget::wipeScreen);


	connect(audioSettingsWidget, &AudioSettingsWidget::outputDeviceSelected, this, [scopeWidget](const QAudioDeviceInfo& audioDeviceInfo){
		scopeWidget->setOutputDeviceInfo(audioDeviceInfo);
	});

	connect(audioSettingsWidget, &AudioSettingsWidget::outputVolumeChanged, scopeWidget, &ScopeWidget::setAudioVolume);

	scopeWidget->setBrightness(66.0);
	scopeWidget->setFocus(40.0);
	scopeWidget->setPersistence(48);
}

MainWindow::~MainWindow()
{
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	auto mimeData = event->mimeData();
	if(mimeData->hasText()) {
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
