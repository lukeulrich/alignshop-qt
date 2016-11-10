# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Oct 25 15:28:22 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestTaskTreeFunctions
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../TaskTreeFunctions.h \
           ../Task.h \
           ../ITask.h
SOURCES += TestTaskTreeFunctions.cpp \
           ../TaskTreeFunctions.cpp \
           ../Task.cpp

DEFINES += TESTING
