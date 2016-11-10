# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 15:57:14 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

QT -= gui
TARGET = MaxHBonds
DEPENDPATH += .
INCLUDEPATH += .

SOURCES += MaxHBonds.cpp \
           ../DimerScoreCalculator.cpp \
           ../../core/BioString.cpp \
           ../../core/util/ClosedIntRange.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp

