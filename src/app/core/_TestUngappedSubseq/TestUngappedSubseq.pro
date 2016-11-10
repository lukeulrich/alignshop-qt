# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jun 13 12:34:52 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestUngappedSubseq
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../UngappedSubseq.h
SOURCES += TestUngappedSubseq.cpp \
           ../UngappedSubseq.cpp \
           ../Seq.cpp \
           ../BioString.cpp \
           ../constants.cpp \
           ../misc.cpp

DEFINES += TESTING
