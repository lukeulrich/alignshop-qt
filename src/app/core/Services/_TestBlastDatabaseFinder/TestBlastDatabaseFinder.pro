# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Sep 26 15:17:13 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestBlastDatabaseFinder
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../BlastDatabaseFinder.h \
           ../BlastDbCmdBase.h \
           ../AbstractProcessWrapper.h
SOURCES += TestBlastDatabaseFinder.cpp \
           ../BlastDatabaseFinder.cpp \
           ../BlastDbCmdBase.cpp \
           ../AbstractProcessWrapper.cpp \
           ../../constants.cpp

DEFINES += TESTING
