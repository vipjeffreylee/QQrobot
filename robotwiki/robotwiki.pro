#-------------------------------------------------
#
# Project created by QtCreator 2013-09-22T15:28:29
#
#-------------------------------------------------

QT       +=sql network
QT       -= gui
CONFIG   +=C++11
TEMPLATE = lib

DEFINES += ROBOT_LIBRARY

INCLUDEPATH    += ../QQ
SOURCES += robot.cpp \
    chinaweather.cpp \
    baiduweather.cpp \
    baidutranslate.cpp

HEADERS += robot.h \
    chinaweather.h \
    baiduweather.h \
    baidutranslate.h

TARGET          = $$qtLibraryTarget(robotwiki)
DESTDIR         = ../plugins

EXAMPLE_FILES = robotwikiplugin.json

OTHER_FILES += \

