#-------------------------------------------------
#
# Project created by QtCreator 2016-12-05T20:07:37
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StacieEmu
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp

HEADERS  += mainwindow.h \
    settingsdialog.h \
    console.h

FORMS    += mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    stacieemu.qrc
