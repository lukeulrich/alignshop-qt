QT -= gui
QT += testlib
CONFIG += console debug
TARGET = TestMpttNode

INCLUDEPATH += . ../..
DEPENDPATH += . ../..

SOURCES += TestMpttNode.cpp MpttNode.cpp
