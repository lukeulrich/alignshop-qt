#-------------------------------------------------
#
# Project created by QtCreator 2010-06-21T12:11:09
#
#-------------------------------------------------

QT       += testlib sql

TARGET = TestDbDataSource
CONFIG   += console debug
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += . ../.. ../../exceptions
DEPENDPATH += . ../.. ../../exceptions
SOURCES += TestDbDataSource.cpp \
           DbDataSource.cpp \
           DataRow.cpp \
           global.cpp
