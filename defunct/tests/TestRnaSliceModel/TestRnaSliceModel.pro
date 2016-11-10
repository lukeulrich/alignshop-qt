CONFIG += qtestlib debug
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += SliceModel.h AdocTreeModel.h RnaSliceModel.h
SOURCES += TestRnaSliceModel.cpp RnaSliceModel.cpp SliceModel.cpp AdocTreeModel.cpp DbDataSource.cpp AdocDbDataSource.cpp global.cpp DataRow.cpp AdocTreeNode.cpp MpttTreeConverter.cpp

DEFINES += TESTING
