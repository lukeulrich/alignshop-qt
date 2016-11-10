# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Aug 26 16:34:08 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestNNStructureTool
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../NNStructureTool.h
SOURCES += TestNNStructureTool.cpp \
           ../NNStructureTool.cpp \
           ../FannWrapper.cpp \
           ../../Parsers/PssmParser.cpp \
           ../../util/PssmUtil.cpp \
           ../../constants.cpp

unix {
    LIBS += -L../../../../fann-2.1.0/src/.libs -lfann
}

DEFINES += TESTING
