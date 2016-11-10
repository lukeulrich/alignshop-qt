# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed Dec 15 12:54:25 2010)
#
# Copyright (C) 2010 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestBioSymbol
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += 
SOURCES += TestBioSymbol.cpp BioSymbol.cpp global.cpp

DEFINES += TESTING
