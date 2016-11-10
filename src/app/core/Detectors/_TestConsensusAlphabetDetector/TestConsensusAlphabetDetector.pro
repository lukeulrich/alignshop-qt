# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed May 25 13:11:22 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestConsensusAlphabetDetector
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += ConsensusAlphabetDetector.h
SOURCES += TestConsensusAlphabetDetector.cpp \
           ../AlphabetDetector.cpp \
           ../BioStringValidator.cpp \
           ../BioString.cpp \
           ../constants.cpp \
           ../misc.cpp

DEFINES += TESTING
