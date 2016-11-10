QT += sql
CONFIG += debug qtestlib
TEMPLATE = app
DEFINES += TESTING

INCLUDEPATH += . ../.. ../../models
DEPENDPATH += . ../.. ../../models

HEADERS += AbstractAdocDataSource.h \
           SynchronousAdocDataSource.h \
           TableModel.h \
           RelatedTableModel.h \
           Msa.h
SOURCES += TestRelatedTableModel.cpp \
           RelatedTableModel.cpp \
           TableModel.cpp \
           SynchronousAdocDataSource.cpp \
           global.cpp \
           DataRow.cpp \
           MpttNode.cpp \
           AdocTreeNode.cpp \
           MpttTreeConverter.cpp \
           TagGenerator.cpp \
           SqlBuilder.cpp \
           DbAnonSeqFactory.cpp \
           AnonSeqFactory.cpp \
           BioString.cpp \
           AnonSeq.cpp \
           Msa.cpp \
           Subseq.cpp
