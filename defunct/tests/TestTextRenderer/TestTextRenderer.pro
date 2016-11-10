# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Feb  1 16:15:41 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestTextRenderer
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += TextRenderer.h
SOURCES += TestTextRenderer.cpp \
           TextRenderer.cpp \
           util/CharMetrics.cpp \
           util/CharMetricsF.cpp \
           global.cpp

DEFINES += TESTING
