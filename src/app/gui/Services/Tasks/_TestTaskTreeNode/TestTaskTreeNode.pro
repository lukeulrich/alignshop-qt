# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Aug 29 12:45:50 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestTaskTreeNode
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../TaskTreeNode.h \
           ../Task.h \
           ../ITask.h
SOURCES += TestTaskTreeNode.cpp \
           ../TaskTreeNode.cpp \
           ../Task.cpp

DEFINES += TESTING
