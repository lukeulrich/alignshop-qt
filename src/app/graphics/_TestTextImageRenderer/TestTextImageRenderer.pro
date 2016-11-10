# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jun 27 12:23:14 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestTextImageRenderer
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../TextImageRenderer.h \
           ../AbstractTextRenderer.h
SOURCES += TestTextImageRenderer.cpp \
           ../TextImageRenderer.cpp \
           ../AbstractTextRenderer.cpp \
           ../AbstractCharPixelMetrics.cpp \
           ../CharPixelMetrics.cpp \
           ../../core/constants.cpp \
           ../graphics_misc.cpp

DEFINES += TESTING
