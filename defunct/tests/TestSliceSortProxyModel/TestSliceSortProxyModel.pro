CONFIG += qtestlib debug
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += SliceProxyModel.h AdocTreeModel.h SliceSortProxyModel.h
SOURCES += TestSliceSortProxyModel.cpp SliceSortProxyModel.cpp SliceProxyModel.cpp AdocTreeModel.cpp DataRow.cpp AdocTreeNode.cpp global.cpp  BioString.cpp TagGenerator.cpp AnonSeq.cpp

DEFINES += TESTING
