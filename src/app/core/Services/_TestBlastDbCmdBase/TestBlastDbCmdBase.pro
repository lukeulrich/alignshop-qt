# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Nov  7 09:49:27 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestBlastDbCmdBase
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../BlastDbCmdBase.h \
           ../AbstractProcessWrapper.h
SOURCES += TestBlastDbCmdBase.cpp \
           ../BlastDbCmdBase.cpp \
           ../AbstractProcessWrapper.cpp

DEFINES += TESTING
