QT -= gui
QT += testlib
CONFIG += console debug
TARGET = TestMpttTreeConverter
INCLUDEPATH += . ../..
DEPENDPATH += . ../..
SOURCES += TestMpttTreeConverter.cpp MpttTreeConverter.cpp
