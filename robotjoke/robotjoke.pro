#-------------------------------------------------
#
# Project created by QtCreator 2013-09-22T15:28:29
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TEMPLATE = lib

DEFINES += ROBOT_LIBRARY

INCLUDEPATH    += ../QQ
SOURCES += robot.cpp

HEADERS += robot.h
TARGET          = $$qtLibraryTarget(robotjoke)
DESTDIR         = ../plugins

EXAMPLE_FILES = robotjokeplugin.json

