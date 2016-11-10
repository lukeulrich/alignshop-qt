CONFIG += qtestlib debug
TEMPLATE = app
TARGET =
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

# Input
HEADERS += AbstractAdocDataSource.h \
           SynchronousAdocDataSource.h \
           Msa.h
SOURCES += TestDbAnonSeqFactory.cpp \
           DbAnonSeqFactory.cpp \
           AnonSeqFactory.cpp \
           AnonSeq.cpp \
           BioString.cpp \
           AminoString.cpp \
           DnaString.cpp \
           global.cpp \
           SynchronousAdocDataSource.cpp \
           AdocTreeNode.cpp \
           MpttTreeConverter.cpp \
           MpttNode.cpp \
           DataRow.cpp \
           SqlBuilder.cpp \
           Msa.cpp \
           Subseq.cpp

QT += sql
