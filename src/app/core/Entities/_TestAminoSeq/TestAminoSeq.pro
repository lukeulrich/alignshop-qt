# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon May  9 12:38:35 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestAminoSeq
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += AminoSeq.h
SOURCES += TestAminoSeq.cpp \
           AminoSeq.cpp \
           AbstractSeqRecord.cpp

DEFINES += TESTING
