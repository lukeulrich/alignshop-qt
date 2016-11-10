# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed Nov 16 14:52:22 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestOptionSet
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../OptionSet.h
SOURCES += TestOptionSet.cpp \
           ../OptionSet.cpp

DEFINES += TESTING
