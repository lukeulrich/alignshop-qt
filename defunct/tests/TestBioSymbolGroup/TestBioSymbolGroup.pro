# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed Dec 15 18:43:19 2010)
#
# Copyright (C) 2010 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestBioSymbolGroup
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += 
SOURCES += TestBioSymbolGroup.cpp BioSymbolGroup.cpp BioSymbol.cpp global.cpp

DEFINES += TESTING
