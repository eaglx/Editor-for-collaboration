#-------------------------------------------------
#
# Project created by QtCreator 2018-01-31T15:01:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    connectdatamanage.cpp \
    serializedata.cpp

CONFIG += c++11

HEADERS  += mainwindow.h \
    connectdatamanage.h

FORMS    += mainwindow.ui
