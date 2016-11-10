# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jan  3 11:53:50 2011)
#
# Copyright (C) 2010 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT += gui
TARGET = TestAbstractCharPixmapProvider
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += AbstractCharPixmapProvider.h
SOURCES += TestAbstractCharPixmapProvider.cpp AbstractCharPixmapProvider.cpp

DEFINES += TESTING
