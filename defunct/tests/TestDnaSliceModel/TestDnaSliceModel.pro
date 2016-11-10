CONFIG += qtestlib debug
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += SliceModel.h AdocTreeModel.h DnaSliceModel.h
SOURCES += TestDnaSliceModel.cpp DnaSliceModel.cpp SliceModel.cpp AdocTreeModel.cpp DbDataSource.cpp AdocDbDataSource.cpp global.cpp DataRow.cpp AdocTreeNode.cpp MpttTreeConverter.cpp

DEFINES += TESTING
