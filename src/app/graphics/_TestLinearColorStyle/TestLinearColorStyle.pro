# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Aug 29 23:01:49 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestLinearColorStyle
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../LinearColorStyle.h
SOURCES += TestLinearColorStyle.cpp \
           ../LinearColorStyle.cpp

DEFINES += TESTING
