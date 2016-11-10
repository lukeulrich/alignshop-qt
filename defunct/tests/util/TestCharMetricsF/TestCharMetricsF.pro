# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed Feb  9 12:40:31 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestCharMetricsF
DEPENDPATH += . ../../..
INCLUDEPATH += . ../../..

HEADERS += util/CharMetricsF.h
SOURCES += TestCharMetricsF.cpp \
           util/CharMetricsF.cpp \
           util/Rect.cpp \
           global.cpp

DEFINES += TESTING
