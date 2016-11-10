# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jan 24 14:09:49 2011)
#
# Copyright (C) 2011 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestRect
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS +=
SOURCES += TestRect.cpp Rect.cpp

DEFINES += TESTING
