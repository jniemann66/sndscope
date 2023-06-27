# Copyright (C) 2020 Judd Niemann - All Rights Reserved.
# You may use, distribute and modify this code under the
# terms of the GNU Lesser General Public License, version 2.1
#
# You should have received a copy of GNU Lesser General Public License v2.1
# with this file. If not, please refer to: https://github.com/jniemann66/sndscope

QT += core gui multimedia widgets

CONFIG += c++17

# force -O3
# QMAKE_CXXFLAGS_RELEASE -= -O2
# QMAKE_CXXFLAGS_RELEASE += -O3
# QMAKE_LFLAGS_RELEASE -= -O1
# --

#import libsndfile
unix!macx {
    LIBS += -L/usr/lib/x86_64-linux-gnu/ -lsndfile
    INCLUDEPATH += /usr/include
}

macx {
    LIBS *= -L/usr/local/lib -lsndfile
    INCLUDEPATH *= /usr/local/include
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
    audiocontroller.cpp \
    audiosettingswidget.cpp \
    displaysettingswidget.cpp \
    main.cpp \
    mainwindow.cpp \
    phosphor.cpp \
    scopewidget.cpp \
    sweepsettingswidget.cpp \
    transportwidget.cpp

HEADERS += \
    audiocontroller.h \
    audiosettingswidget.h \
    differentiator.h \
    displaysettingswidget.h \
    functimer.h \
    mainwindow.h \
    phosphor.h \
    scopewidget.h \
    sweepparameters.h \
    sweepsettingswidget.h \
    transportwidget.h

TRANSLATIONS += \
    sndscope_en_AU.ts

RESOURCES += \
    config.qrc \
    icons.qrc
