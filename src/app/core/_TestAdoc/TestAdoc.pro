# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed May 18 12:31:20 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
QT += sql
TARGET = TestAdoc
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += Adoc.h
SOURCES += TestAdoc.cpp \
           Adoc.cpp \
           core/DataSources/SqliteAdocSource.cpp \
           core/DataSources/AbstractDbSource.cpp \
           core/BioString.cpp \
           core/Seq.cpp \
           core/DataMappers/AminoSeqMapper.cpp \
           core/DataSources/Crud/DbAstringCrud.cpp \
           core/DataSources/Crud/DbAminoSeqCrud.cpp \
           core/MpttNode.cpp \
           core/MpttTreeConverter.cpp \
           core/misc.cpp \
           core/Entities/AminoSeq.cpp \
           core/Entities/Astring.cpp \
           core/Entities/AbstractSeqRecord.cpp \
           core/constants.cpp

DEFINES += TESTING
LIBS += -lsqlite3
