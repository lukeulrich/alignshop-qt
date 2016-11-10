QT       += testlib sql
TARGET = TestMsaSubseqModel
CONFIG   += debug console
TEMPLATE = app
INCLUDEPATH += . ../.. ../../models
DEPENDPATH += . ../.. ../../models
HEADERS += MsaSubseqModel.h TableModel.h AbstractDataSource.h Msa.h AbstractAdocDataSource.h SynchronousAdocDataSource.h RelatedTableModel.h
SOURCES += TestMsaSubseqModel.cpp MsaSubseqModel.cpp TableModel.cpp DataRow.cpp Msa.cpp Subseq.cpp BioString.cpp global.cpp AnonSeq.cpp TagGenerator.cpp SynchronousAdocDataSource.cpp DbAnonSeqFactory.cpp MpttNode.cpp RelatedTableModel.cpp AdocTreeNode.cpp MpttTreeConverter.cpp AnonSeqFactory.cpp SqlBuilder.cpp
