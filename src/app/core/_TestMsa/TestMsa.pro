# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Jun 21 11:50:00 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestMsa
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../Msa.h
SOURCES += TestMsa.cpp \
           ../Msa.cpp \
           ../BioString.cpp \
           ../UngappedSubseq.cpp \
           ../Subseq.cpp \
           ../Seq.cpp \
           ../constants.cpp \
           ../misc.cpp \
           ../Entities/AminoSeq.cpp \
           ../Entities/Astring.cpp \
           ../Entities/AbstractBasicEntity.cpp \
           ../Entities/AbstractSeq.cpp

DEFINES += TESTING
