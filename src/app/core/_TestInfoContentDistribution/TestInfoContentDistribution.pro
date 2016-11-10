# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Jul 26 15:39:24 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestInfoContentDistribution
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../InfoContentDistribution.h
SOURCES += TestInfoContentDistribution.cpp \
           ../InfoContentDistribution.cpp \
           ../CharCountDistribution.cpp \
           ../misc.cpp \
           ../constants.cpp

DEFINES += TESTING
