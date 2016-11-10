# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Feb  1 19:52:47 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestTextPixmapRenderer
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += TextPixmapRenderer.h \
           TextRenderer.h \
           TextImageRenderer.h
SOURCES += TestTextPixmapRenderer.cpp \
           TextPixmapRenderer.cpp \
           TextImageRenderer.cpp \
           TextRenderer.cpp \
           util/CharMetrics.cpp

DEFINES += TESTING
