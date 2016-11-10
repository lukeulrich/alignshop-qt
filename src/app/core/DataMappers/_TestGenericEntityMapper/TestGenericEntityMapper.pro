# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon May  9 15:31:01 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
QT += sql
TARGET = TestGenericEntityMapper
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../GenericEntityMapper.h
SOURCES += TestGenericEntityMapper.cpp \
           ../../Entities/AbstractSeq.cpp \
           ../../Entities/AbstractMsa.cpp \
           ../../Entities/AbstractBasicEntity.cpp \
           ../../Entities/AminoSeq.cpp \
           ../../Entities/Astring.cpp \
           ../../Entities/DnaSeq.cpp \
           ../../Entities/Dstring.cpp \
           ../../DataSources/AbstractDbSource.cpp \
           ../../DataSources/SqliteAdocSource.cpp \
           ../../BioString.cpp \
           ../../Seq.cpp \
           ../../Subseq.cpp \
           ../../Msa.cpp \
           ../../UngappedSubseq.cpp \
           ../../MpttNode.cpp \
           ../../MpttTreeConverter.cpp \
           ../../misc.cpp \
           ../../constants.cpp \
           ../../DataSources/Crud/DbAstringCrud.cpp \
           ../../DataSources/Crud/DbAminoSeqCrud.cpp \
           ../../DataSources/Crud/DbAminoMsaCrud.cpp \
           ../../DataSources/Crud/DbDstringCrud.cpp \
           ../../DataSources/Crud/DbDnaSeqCrud.cpp

DEFINES += TESTING

LIBS += -lsqlite3
