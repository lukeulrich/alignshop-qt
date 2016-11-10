CONFIG += qtestlib debug
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += SliceProxyModel.h AdocTreeModel.h AminoSliceModel.h SubseqSliceModel.h AdocDataSource.h RelatedTableModel.h TableModel.h
SOURCES += TestAminoSliceModel.cpp AminoSliceModel.cpp SliceProxyModel.cpp AdocTreeModel.cpp AdocDataSource.cpp global.cpp AdocTreeNode.cpp SubseqSliceModel.cpp TableModel.cpp RelatedTableModel.cpp TagGenerator.cpp DataRow.cpp MpttNode.cpp MpttTreeConverter.cpp SqlBuilder.cpp

DEFINES += TESTING
