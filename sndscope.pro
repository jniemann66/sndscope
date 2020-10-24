# Copyright (C) 2020 Judd Niemann - All Rights Reserved.
# You may use, distribute and modify this code under the
# terms of the GNU Lesser General Public License, version 2.1
#
# You should have received a copy of GNU Lesser General Public License v2.1
# with this file. If not, please refer to: https://github.com/jniemann66/sndscope

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

#disable deprecated stuff (won't compile)
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#import libsndfile
unix {
    LIBS += -L/usr/lib/x86_64-linux-gnu/ -lsndfile
    INCLUDEPATH += /usr/include
}

win32 {
    LIBSNDFILEPATH = $$PWD/libsndfile-1.0.30-win64
    message(libsndfile path: $$LIBSNDFILEPATH)
    LIBS += -L$${LIBSNDFILEPATH}/lib -lsndfile
    INCLUDEPATH += $${LIBSNDFILEPATH}/include

    #copy libsndfile dll to build folder
    CONFIG += file_copies
    COPIES += dlls
    dlls.files = $$files($${LIBSNDFILEPATH}/bin/sndfile.dll)
    dlls.path = $$OUT_PWD
}


SOURCES += \
    displaysettingswidget.cpp \
    main.cpp \
    mainwindow.cpp \
    scopewidget.cpp \
    transportwidget.cpp

HEADERS += \
    displaysettingswidget.h \
    mainwindow.h \
    scopewidget.h \
    transportwidget.h

TRANSLATIONS += \
    sndscope_en_AU.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
