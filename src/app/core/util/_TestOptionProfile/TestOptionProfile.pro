# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed Nov 16 11:18:16 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestOptionProfile
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../OptionProfile.h
SOURCES += TestOptionProfile.cpp \
           ../OptionProfile.cpp

DEFINES += TESTING
