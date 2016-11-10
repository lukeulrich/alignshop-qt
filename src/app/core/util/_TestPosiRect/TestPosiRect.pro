# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Feb 21 11:02:40 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPosiRect
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += PosiRect.h
SOURCES += TestPosiRect.cpp \
           Rect.cpp

DEFINES += TESTING
