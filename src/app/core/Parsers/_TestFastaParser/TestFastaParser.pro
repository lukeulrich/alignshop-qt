# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon May 23 15:10:04 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestFastaParser
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += FastaParser.h \
           ISequenceParser.h
SOURCES += TestFastaParser.cpp \
           FastaParser.cpp \
           ../AbstractSequenceParser.cpp \
           ../BioString.cpp \
           ../constants.cpp \
           ../misc.cpp

DEFINES += TESTING
