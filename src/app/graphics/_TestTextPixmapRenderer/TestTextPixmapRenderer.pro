# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jun 27 12:37:33 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestTextPixmapRenderer
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../TextPixmapRenderer.h \
           ../AbstractTextRenderer.h \
           ../TextImageRenderer.h
SOURCES += TestTextPixmapRenderer.cpp \
           ../TextImageRenderer.cpp \
           ../TextPixmapRenderer.cpp \
           ../AbstractTextRenderer.cpp \
           ../AbstractCharPixelMetrics.cpp \
           ../CharPixelMetrics.cpp \
           ../../core/constants.cpp \
           ../graphics_misc.cpp

DEFINES += TESTING
