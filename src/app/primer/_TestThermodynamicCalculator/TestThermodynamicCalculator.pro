# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Jan 10 12:23:27 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestThermodynamicCalculator
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../ThermodynamicCalculator.h
SOURCES += TestThermodynamicCalculator.cpp \
           ../ThermodynamicCalculator.cpp \
           ../ThermodynamicConstants.cpp \
           ../../core/BioString.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp

DEFINES += TESTING
