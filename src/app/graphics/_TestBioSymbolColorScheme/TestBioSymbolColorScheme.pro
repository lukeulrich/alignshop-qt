# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jun 27 17:47:20 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestBioSymbolColorScheme
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../BioSymbolColorScheme.h
SOURCES += TestBioSymbolColorScheme.cpp \
           ../CharColorScheme.cpp \
           ../BioSymbolColorScheme.cpp \
           ../../core/BioSymbolGroup.cpp \
           ../../core/BioSymbol.cpp

DEFINES += TESTING
