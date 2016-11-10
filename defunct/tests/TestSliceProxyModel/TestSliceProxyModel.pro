CONFIG += qtestlib debug
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += SliceProxyModel.h AdocTreeModel.h MockSliceProxyModel.h Msa.h
SOURCES += TestSliceProxyModel.cpp SliceProxyModel.cpp AdocTreeModel.cpp global.cpp DataRow.cpp AdocTreeNode.cpp BioString.cpp AnonSeq.cpp Msa.cpp Subseq.cpp TagGenerator.cpp

DEFINES += TESTING
