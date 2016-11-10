# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Jun 24 17:09:01 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestBioSymbolGroup
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../BioSymbolGroup.h
SOURCES += TestBioSymbolGroup.cpp \
           ../BioSymbolGroup.cpp \
           ../BioSymbol.cpp

DEFINES += TESTING
