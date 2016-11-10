# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 22:32:27 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPrimerPair
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../PrimerPair.h
SOURCES += TestPrimerPair.cpp \
           ../PrimerPair.cpp \
           ../Primer.cpp \
           ../PrimerFactory.cpp \
           ../ThermodynamicCalculator.cpp \
           ../ThermodynamicConstants.cpp \
           ../DimerScoreCalculator.cpp \
           ../../core/BioString.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp

DEFINES += TESTING
