QT += sql xmlpatterns
CONFIG += qtestlib debug
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += AdocTreeModel.h
SOURCES += TestAdocTreeModel.cpp AdocTreeModel.cpp AdocTreeNode.cpp

DEFINES += TESTING
