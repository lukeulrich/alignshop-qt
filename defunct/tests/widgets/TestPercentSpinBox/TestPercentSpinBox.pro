# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Feb 11 10:58:10 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestPercentSpinBox
DEPENDPATH += . ../../../widgets
INCLUDEPATH += . ../../../widgets

HEADERS += PercentSpinBox.h AgDoubleSpinBox.h
SOURCES += TestPercentSpinBox.cpp

DEFINES += TESTING
