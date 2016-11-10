# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jan 31 15:38:38 2011)
#
# Copyright (C) 111  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestSymbolColorScheme
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += CharColorScheme.h
SOURCES += TestSymbolColorScheme.cpp SymbolColorScheme.cpp CharColorScheme.cpp

DEFINES += TESTING
