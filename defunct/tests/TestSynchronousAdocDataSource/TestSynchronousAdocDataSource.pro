QT += sql
DEFINES += TESTING
CONFIG += debug qtestlib
TEMPLATE = app

INCLUDEPATH += . ../..
DEPENDPATH += . ../..

HEADERS += AbstractAdocDataSource.h SynchronousAdocDataSource.h Msa.h
SOURCES += TestSynchronousAdocDataSource.cpp SynchronousAdocDataSource.cpp MpttTreeConverter.cpp MpttNode.cpp AdocTreeNode.cpp global.cpp DataRow.cpp SqlBuilder.cpp BioString.cpp DbAnonSeqFactory.cpp AnonSeq.cpp AnonSeqFactory.cpp Msa.cpp Subseq.cpp AminoString.cpp DnaString.cpp RnaString.cpp
