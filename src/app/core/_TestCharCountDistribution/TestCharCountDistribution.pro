# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jun 27 12:59:41 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestCharCountDistribution
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../CharCountDistribution.h
SOURCES += TestCharCountDistribution.cpp \
           ../CharCountDistribution.cpp \
           ../misc.cpp \
           ../constants.cpp

DEFINES += TESTING
