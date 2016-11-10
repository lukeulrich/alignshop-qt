# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Jun 28 15:17:52 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestSymbolColorScheme
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../SymbolColorScheme.h
SOURCES += TestSymbolColorScheme.cpp \
           ../SymbolColorScheme.cpp \
           ../CharColorScheme.cpp

DEFINES += TESTING
