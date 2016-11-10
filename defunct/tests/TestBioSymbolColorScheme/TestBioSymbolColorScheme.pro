# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Dec 20 13:49:18 2010)
#
# Copyright (C) 2010 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT += gui
TARGET = TestBioSymbolColorScheme
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += 
SOURCES += TestBioSymbolColorScheme.cpp BioSymbolColorScheme.cpp CharColorScheme.cpp BioSymbolGroup.cpp BioSymbol.cpp

DEFINES += TESTING
