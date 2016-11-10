include(../common.pri)

QT       += core
QT       -= gui

TARGET = version-tool
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

DESTDIR = ./
