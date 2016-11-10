# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Oct  4 09:28:00 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestTarUtil
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../TarUtil.h
SOURCES += TestTarUtil.cpp \
           ../TarUtil.cpp

DEFINES += TESTING
