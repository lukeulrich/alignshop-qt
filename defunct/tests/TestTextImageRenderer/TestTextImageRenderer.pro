# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Feb  1 17:16:47 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestTextImageRenderer
DEPENDPATH += . ../..
INCLUDEPATH += . ../..
QT += gui
HEADERS += TextRenderer.h \
           TextImageRenderer.h
SOURCES += TestTextImageRenderer.cpp \
           TextImageRenderer.cpp \
           TextRenderer.cpp \
           util/Rect.cpp \
           util/CharMetrics.cpp \
           global.cpp

DEFINES += TESTING
