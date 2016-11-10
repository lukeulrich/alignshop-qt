# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Aug 26 12:49:59 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPssmParser
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../PssmParser.h
SOURCES += TestPssmParser.cpp \
           ../PssmParser.cpp

DEFINES += TESTING
