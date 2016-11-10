# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jan  3 12:22:31 2011)
#
# Copyright (C) 2011 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT += gui
TARGET = TestFontCharPixmapProvider
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += AbstractCharPixmapProvider.h \
           FontCharPixmapProvider.h
SOURCES += TestFontCharPixmapProvider.cpp \
           FontCharPixmapProvider.cpp \
           AbstractCharPixmapProvider.cpp \
           global.cpp

DEFINES += TESTING
