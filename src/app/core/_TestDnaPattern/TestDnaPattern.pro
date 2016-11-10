# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 10:47:53 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestDnaPattern
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../DnaPattern.h
SOURCES += TestDnaPattern.cpp \
           ../DnaPattern.cpp \
           ../BioString.cpp \
           ../misc.cpp \
           ../constants.cpp

DEFINES += TESTING
