# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 18:15:28 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestRestrictionEnzyme
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../RestrictionEnzyme.h
SOURCES += TestRestrictionEnzyme.cpp \
           ../../core/BioString.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp


DEFINES += TESTING
