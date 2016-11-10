# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu May 12 10:53:16 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestMpttTreeConverter
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += MpttTreeConverter.h
SOURCES += TestMpttTreeConverter.cpp \
           ../MpttNode.cpp  \
           ../MpttTreeConverter.cpp

DEFINES += TESTING
