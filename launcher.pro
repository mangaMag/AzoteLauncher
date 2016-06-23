#-------------------------------------------------
#
# Project created by QtCreator 2014-08-06T23:31:56
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AzendarUpdater
TEMPLATE = app

DEFINES += LAUNCHER_VERSION=3

ICON = ressources/Launcher.icns
RC_ICONS = ressources/azendaricone.ico

SOURCES += main.cpp\
    gui/launcher.cpp \
    updater/updater.cpp \
    logger/logger.cpp \
    http/http.cpp \
    gui/console.cpp \
    updater/selfupdater.cpp \
    others/sound.cpp \
    utils/system.cpp \
    gui/settings.cpp \
    dep/SingleApplication/singleapplication.cpp

HEADERS  += gui/launcher.h \
    updater/updater.h \
    logger/logger.h \
    serialization/singleton.h \
    http/http.h \
    gui/console.h \
    updater/selfupdater.h \
    others/sound.h \
    utils/system.h \
    logger/loglevel.h \
    gui/settings.h \
    dep/SingleApplication/singleapplication.h

FORMS    += gui/launcher.ui \
    gui/console.ui \
    gui/settings.ui

RESOURCES += \
    ressources.qrc
