# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Aug 26 13:36:58 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPssmUtil
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../PssmUtil.h
SOURCES += TestPssmUtil.cpp \
           ../PssmUtil.cpp \
           ../../Parsers/PssmParser.cpp

DEFINES += TESTING
