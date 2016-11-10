# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 15:57:14 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestDimerScoreCalculator
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../DimerScoreCalculator.h
SOURCES += TestDimerScoreCalculator.cpp \
           ../DimerScoreCalculator.cpp \
           ../../core/BioString.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp

DEFINES += TESTING
