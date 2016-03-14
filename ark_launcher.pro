#-------------------------------------------------
#
# Project created by QtCreator 2014-08-06T23:31:56
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ark_launcher
TEMPLATE = app

SOURCES += main.cpp\
        gui/launcher.cpp \
    updater/updater.cpp \
    logger/logger.cpp \
    http/http.cpp \
    gui/console.cpp

HEADERS  += gui/launcher.h \
    updater/updater.h \
    logger/logger.h \
    serialization/singleton.h \
    http/http.h \
    gui/console.h

FORMS    += gui/launcher.ui \
    gui/console.ui

RESOURCES += \
    ressources.qrc
