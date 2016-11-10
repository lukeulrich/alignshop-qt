# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed Jul 27 09:55:28 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestLiveInfoContentDistribution
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../LiveInfoContentDistribution.h \
           ../AbstractLiveCharCountDistribution.h
SOURCES += TestLiveInfoContentDistribution.cpp \
           ../LiveInfoContentDistribution.cpp \
           ../InfoContentDistribution.cpp \
           ../CharCountDistribution.cpp \
           ../misc.cpp \
           ../constants.cpp

DEFINES += TESTING
