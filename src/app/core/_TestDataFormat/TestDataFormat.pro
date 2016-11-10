# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue May 31 10:30:57 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestDataFormat
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../DataFormat.h \
           ../Parsers/ISequenceParser.h
SOURCES += TestDataFormat.cpp \
           ../DataFormat.cpp

DEFINES += TESTING
