#-------------------------------------------------
#
# Project created by QtCreator 2010-06-21T10:09:07
#
#-------------------------------------------------

QT       += testlib sql
QT       -= core gui

TARGET = TestSqlBuilder
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += . ../..
DEPENDPATH += . ../..
SOURCES += TestSqlBuilder.cpp SqlBuilder.cpp
