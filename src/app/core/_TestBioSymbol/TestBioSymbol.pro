# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Jun 24 14:11:43 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestBioSymbol
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../BioSymbol.h
SOURCES += TestBioSymbol.cpp \
           ../BioSymbol.cpp

DEFINES += TESTING
