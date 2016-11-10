# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Apr  7 09:15:06 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestBioString
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += BioString.h
SOURCES += TestBioString.cpp \
           BioString.cpp \
           misc.cpp \
           constants.cpp

