# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Apr  7 21:43:46 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = Testmiscc
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += misc.h
SOURCES += Testmisc.cpp \
           misc.cpp \
           constants.cpp

DEFINES += TESTING
