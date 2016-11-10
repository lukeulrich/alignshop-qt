# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Nov  4 15:41:41 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestBlastSequenceFetcher
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../BlastSequenceFetcher.h \
           ../BlastDbCmdBase.h \
           ../AbstractProcessWrapper.h
SOURCES += TestBlastSequenceFetcher.cpp \
           ../BlastDbCmdBase.cpp \
           ../AbstractProcessWrapper.cpp \
           ../../BioString.cpp \
           ../../misc.cpp \
           ../../constants.cpp \
           ../BlastSequenceFetcher.cpp

DEFINES += TESTING
