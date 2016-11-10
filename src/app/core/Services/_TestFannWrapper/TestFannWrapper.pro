# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Aug 25 17:02:59 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestFannWrapper
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../FannWrapper.h
SOURCES += TestFannWrapper.cpp \
           ../FannWrapper.cpp

DEFINES += TESTING

unix {
    LIBS += -L../../../../fann-2.1.0/src/.libs -lfann
}
