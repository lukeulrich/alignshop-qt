# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon May  9 11:27:42 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestAbstractSeqRecord
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += AbstractSeqRecord.h
SOURCES += TestAbstractSeqRecord.cpp \
           AbstractSeqRecord.cpp \
           ../../Seq.cpp \
           ../../BioString.cpp \
           ../../misc.cpp \
           ../../constants.cpp

DEFINES += TESTING
