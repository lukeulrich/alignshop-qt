# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu May 12 10:20:49 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestMpttNode
DEPENDPATH += .
INCLUDEPATH += .

SOURCES += TestMpttNode.cpp \
           ../MpttNode.cpp

DEFINES += TESTING
