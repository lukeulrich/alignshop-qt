# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Feb  3 11:06:53 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestCharMetrics
DEPENDPATH += . ../../..
INCLUDEPATH += . ../../..

HEADERS += util/CharMetrics.h

SOURCES += TestCharMetrics.cpp \
           util/CharMetricsF.cpp \
           util/CharMetrics.cpp \
           util/Rect.cpp \
           global.cpp

DEFINES += TESTING
