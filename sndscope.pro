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

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    scopewidget.cpp \
    transportwidget.cpp

HEADERS += \
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
