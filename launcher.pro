#-------------------------------------------------
#
# Project created by QtCreator 2014-08-06T23:31:56
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Azote
TEMPLATE = app

DEFINES += LAUNCHER_VERSION=5
DEFINES += URL=\\\"http://updates.azote.us\\\"

ICON = ressources/icon.icns
RC_ICONS = ressources/icon.ico

QMAKE_INFO_PLIST=Info.plist

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
    dep/SingleApplication/singleapplication.cpp \
    gui/server.cpp \
    gui/home.cpp

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
    dep/SingleApplication/singleapplication.h \
    gui/server.h \
    gui/home.h

FORMS    += gui/launcher.ui \
    gui/console.ui \
    gui/settings.ui \
    gui/server.ui \
    gui/home.ui

RESOURCES += \
    ressources.qrc
