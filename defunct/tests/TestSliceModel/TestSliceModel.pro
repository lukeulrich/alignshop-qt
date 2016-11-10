CONFIG += qtestlib debug
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += SliceModel.h AdocTreeModel.h MockSliceModel.h
SOURCES += TestSliceModel.cpp \
           SliceModel.cpp \
           AdocTreeModel.cpp \
           DbDataSource.cpp \
           AdocDbDataSource.cpp \
           global.cpp \
           DataRow.cpp \
           AdocTreeNode.cpp

DEFINES += TESTING
