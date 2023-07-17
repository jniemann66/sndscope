# Copyright (C) 2020 Judd Niemann - All Rights Reserved.
# You may use, distribute and modify this code under the
# terms of the GNU Lesser General Public License, version 2.1
#
# You should have received a copy of GNU Lesser General Public License v2.1
# with this file. If not, please refer to: https://github.com/jniemann66/sndscope

QT += core gui multimedia widgets

CONFIG += c++17

#activate blend2d to do the rendering (instead of QPainter):
#CONFIG += blend2d

AVX2 {
 message(Enabling AVX2)
 QMAKE_CXXFLAGS_RELEASE -= -O2
 QMAKE_CXXFLAGS_RELEASE += -O3
 QMAKE_CXXFLAGS_RELEASE += -mavx2
 message($${QMAKE_CXXFLAGS_RELEASE})
}

#import libsndfile
linux {
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

    blend2d {
        # path to blend2d headers
        BLEND2DINCLUDE = $$clean_path($$PWD/../blend2d/src)

        # path to blend2d library files
        BLEND2DLIB = $$clean_path($$PWD/../blend2dlib)

        message(blend2d include path: $$BLEND2DINCLUDE)
        message(blend2d library path: $$BLEND2DLIB)

        LIBS += -L$${BLEND2DLIB} -lblend2d
        INCLUDEPATH += $${BLEND2DINCLUDE}
        DEFINES += SNDSCOPE_BLEND2D
    }

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
    plotmode.cpp \
    plotmodewidget.cpp \
    plotter.cpp \
    scopewidget.cpp \
    sweepsettingswidget.cpp \
    transportwidget.cpp

HEADERS += \
    audiocontroller.h \
    audiosettingswidget.h \
    blimagewrapper.h \
    delayline.h \
    differentiator.h \
    displaysettingswidget.h \
    functimer.h \
    mainwindow.h \
    movingaverage.h \
    phosphor.h \
    plotmode.h \
    plotmodewidget.h \
    plotter.h \
    scopewidget.h \
    sweepparameters.h \
    sweepsettingswidget.h \
    transportwidget.h \
    upsampler.h

blend2d {
    SOURCES +=  blimagewrapper.cpp
    HEADERS +=  blimagewrapper.h
}

TRANSLATIONS += \
    sndscope_en_AU.ts

RESOURCES += \
    config.qrc \
    icons.qrc
