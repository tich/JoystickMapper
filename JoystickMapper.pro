#-------------------------------------------------
#
# Project created by QtCreator 2013-04-11T17:16:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JoystickMapper
TEMPLATE = app

LIBS += -L$$PWD/windows/
INCLUDEPATH += $$PWD/windows/

LIBS += -lusb-1.0

SOURCES += main.cpp\
        mainwindow.cpp \
    usbhandler.cpp

HEADERS  += mainwindow.h \
    common.h \
    usbhandler.h

FORMS    += mainwindow.ui
