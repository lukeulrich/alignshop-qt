# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon May  9 16:23:25 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
QT += sql
TARGET = TestAstringMapper
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += AstringMapper.h
SOURCES += TestAstringMapper.cpp \
           ../../Entities/AbstractSeqRecord.cpp \
           ../../Entities/Astring.cpp \
           ../../Entities/AminoSeq.cpp \
           ../../Entities/Dstring.cpp \
           ../../Entities/DnaSeq.cpp \
           ../../DataSources/AbstractDbSource.cpp \
           ../../DataSources/SqliteAdocSource.cpp \
           ../../BioString.cpp \
           ../../Seq.cpp \
           ../../UngappedSubseq.cpp \
           ../../misc.cpp \
           ../../constants.cpp \
           ../../MpttNode.cpp \
           ../../MpttTreeConverter.cpp \
           ../../DataSources/Crud/DbAstringCrud.cpp \
           ../../DataSources/Crud/DbAminoSeqCrud.cpp \
           ../../DataSources/Crud/DbDstringCrud.cpp \
           ../../DataSources/Crud/DbDnaSeqCrud.cpp

DEFINES += TESTING

LIBS += -lsqlite3
