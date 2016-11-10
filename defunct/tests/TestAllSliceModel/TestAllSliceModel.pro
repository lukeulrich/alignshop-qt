CONFIG += qtestlib debug
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += SliceModel.h AdocTreeModel.h AllSliceModel.h
SOURCES += TestAllSliceModel.cpp AllSliceModel.cpp SliceModel.cpp AdocTreeModel.cpp DbDataSource.cpp AdocDbDataSource.cpp DataRow.cpp AdocTreeNode.cpp MpttTreeConverter.cpp global.cpp

DEFINES += TESTING
