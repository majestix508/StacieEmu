#-------------------------------------------------
#
# Project created by QtCreator 2016-12-05T20:07:37
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StacieEmu
TEMPLATE = app

RC_ICONS = emu.ico

SOURCES += main.cpp\
        mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    ddldialog.cpp

HEADERS  += mainwindow.h \
    settingsdialog.h \
    console.h \
    ddldialog.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    ddldialog.ui

RESOURCES += \
    stacieemu.qrc
