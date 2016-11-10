# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed May 25 15:40:45 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestAlphabetDetector
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += AlphabetDetector.h
SOURCES += TestAlphabetDetector.cpp \
           ../AlphabetDetector.cpp \
           ../../BioStringValidator.cpp \
           ../../BioString.cpp \
           ../../constants.cpp \
           ../../misc.cpp

DEFINES += TESTING
