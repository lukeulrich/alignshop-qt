# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon May  9 09:40:22 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
QT += sql
TARGET = TestDbAminoSeqCrud
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += DbAminoSeqCrud.h
SOURCES += TestDbAminoSeqCrud.cpp \
           ../../Entities/AbstractSeqRecord.cpp \
           ../../Entities/Astring.cpp \
           ../../Entities/AminoSeq.cpp \
           DbAminoSeqCrud.cpp \
           ../../AbstractDbSource.cpp \
           ../../../Seq.cpp \
           ../../../UngappedSubseq.cpp \
           ../../../BioString.cpp \
           ../../../misc.cpp \
           ../../../constants.cpp

DEFINES += TESTING
