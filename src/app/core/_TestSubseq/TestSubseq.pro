# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Apr  8 16:26:56 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestSubseq
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../Subseq.h
SOURCES += TestSubseq.cpp \
           ../UngappedSubseq.cpp \
           ../Subseq.cpp \
           ../Seq.cpp \
           ../BioString.cpp \
           ../misc.cpp \
           ../constants.cpp

DEFINES += TESTING
