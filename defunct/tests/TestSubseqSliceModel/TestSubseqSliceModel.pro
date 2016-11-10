CONFIG += qtestlib debug
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += SliceProxyModel.h AdocTreeModel.h SubseqSliceModel.h AbstractAdocDataSource.h SynchronousAdocDataSource.h RelatedTableModel.h TableModel.h Msa.h AnonSeqFactory.h
SOURCES += TestSubseqSliceModel.cpp SliceProxyModel.cpp AdocTreeModel.cpp global.cpp AdocTreeNode.cpp SubseqSliceModel.cpp TableModel.cpp RelatedTableModel.cpp TagGenerator.cpp DataRow.cpp MpttNode.cpp MpttTreeConverter.cpp SqlBuilder.cpp \
           SynchronousAdocDataSource.cpp BioString.cpp AnonSeq.cpp Msa.cpp Subseq.cpp DbAnonSeqFactory.cpp AnonSeqFactory.cpp

DEFINES += TESTING
