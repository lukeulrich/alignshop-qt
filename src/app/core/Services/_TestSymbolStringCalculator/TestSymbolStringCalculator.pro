# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Jun 24 18:01:26 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestSymbolStringCalculator
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../SymbolStringCalculator.h
SOURCES += TestSymbolStringCalculator.cpp \
           ../SymbolStringCalculator.cpp \
           ../../BioSymbolGroup.cpp \
           ../../BioSymbol.cpp \
           ../../misc.cpp \
           ../../constants.cpp

DEFINES += TESTING
