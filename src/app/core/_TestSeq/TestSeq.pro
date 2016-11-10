# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Apr  8 15:39:56 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestSeq
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += Seq.h
SOURCES += TestSeq.cpp \
           Seq.cpp \
           BioString.cpp \
           constants.cpp \
           misc.cpp

DEFINES += TESTING
