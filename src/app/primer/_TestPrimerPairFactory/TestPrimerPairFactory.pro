# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 22:27:11 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPrimerPairFactory
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../PrimerPairFactory.h
SOURCES += TestPrimerPairFactory.cpp \
           ../PrimerPairFactory.cpp \
           ../PrimerFactory.cpp \
           ../Primer.cpp \
           ../PrimerPair.cpp \
           ../ThermodynamicCalculator.cpp \
           ../ThermodynamicConstants.cpp \
           ../DimerScoreCalculator.cpp \
           ../../core/BioString.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp

DEFINES += TESTING
