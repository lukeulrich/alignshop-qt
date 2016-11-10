# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jun 27 17:44:05 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestCharColorScheme
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../CharColorScheme.h
SOURCES += TestCharColorScheme.cpp \
           ../CharColorScheme.cpp

DEFINES += TESTING
