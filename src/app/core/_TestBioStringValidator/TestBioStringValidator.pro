# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue May 24 16:09:42 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestBioStringValidator
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += BioStringValidator.h
SOURCES += TestBioStringValidator.cpp \
           BioStringValidator.cpp \
           BioString.cpp \
           misc.cpp \
           constants.cpp

DEFINES += TESTING
