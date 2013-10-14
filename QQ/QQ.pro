#-------------------------------------------------
#
# Project created by QtCreator 2013-08-28T11:14:11
#
#-------------------------------------------------

QT       += core gui\
            network\
            multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += C++11
TARGET = QQrobot
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    webqqnet.cpp \
    logindlg.cpp \
    webqq.cpp \
    qqgroup.cpp \
    qqmsg.cpp \
    qqfriend.cpp

HEADERS  += mainwindow.h \
    webqqnet.h \
    logindlg.h \
    webqq.h \
    qqgroup.h \
    qqmsg.h \
    qqfriend.h \
    robotinterface.h

FORMS    += mainwindow.ui \
    logindlg.ui
win32 {
    CONFIG(debug, release|debug):DESTDIR = ../debug/
    CONFIG(release, release|debug):DESTDIR = ../release/
} else {
    DESTDIR    = ../
}

RESOURCES += \
    imgs.qrc

RC_FILE = myapp.rc
