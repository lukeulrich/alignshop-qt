QT += sql
CONFIG += debug qtestlib
TEMPLATE = app
DEFINES += TESTING

INCLUDEPATH += . ../.. ../../models
DEPENDPATH += . ../.. ../../models

HEADERS += AbstractAdocDataSource.h SynchronousAdocDataSource.h TableModel.h Msa.h
SOURCES += TestTableModel.cpp TableModel.cpp SynchronousAdocDataSource.cpp global.cpp DataRow.cpp MpttNode.cpp AdocTreeNode.cpp MpttTreeConverter.cpp SqlBuilder.cpp TagGenerator.cpp BioString.cpp DbAnonSeqFactory.cpp AnonSeq.cpp AnonSeqFactory.cpp Msa.cpp Subseq.cpp
