# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 21:59:08 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPrimerFactory
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../PrimerFactory.h
SOURCES += TestPrimerFactory.cpp \
           ../PrimerFactory.cpp \
           ../Primer.cpp \
           ../ThermodynamicCalculator.cpp \
           ../ThermodynamicConstants.cpp \
           ../DimerScoreCalculator.cpp \
           ../../core/BioString.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp

DEFINES += TESTING
