# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed Oct 26 17:20:15 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestMptt
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../Mptt.h
SOURCES += TestMptt.cpp

DEFINES += TESTING
