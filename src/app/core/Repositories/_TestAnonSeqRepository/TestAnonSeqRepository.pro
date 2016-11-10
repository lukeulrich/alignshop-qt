# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue May 10 10:59:40 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
QT += sql
TARGET = TestAnonSeqRepository
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += AnonSeqRepository.h
SOURCES += TestAnonSeqRepository.cpp \
           ../../Entities/AbstractSeqRecord.cpp \
           ../../Entities/Astring.cpp \
           ../../Entities/AminoSeq.cpp \
           ../../DataSources/AbstractDbSource.cpp \
           ../../DataSources/SqliteAdocSource.cpp \
           ../../Seq.cpp \
           ../../BioString.cpp \
           ../../misc.cpp \
           ../../constants.cpp \
           ../../DataSources/Crud/DbAstringCrud.cpp \
           ../../DataSources/Crud/DbAminoSeqCrud.cpp \
           ../../MpttNode.cpp \
           ../../MpttTreeConverter.cpp

DEFINES += TESTING
