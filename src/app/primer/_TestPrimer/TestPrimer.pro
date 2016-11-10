# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 17:50:31 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPrimer
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../Primer.h
SOURCES += TestPrimer.cpp \
           ../Primer.cpp \
           ../../core/BioString.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp

DEFINES += TESTING
