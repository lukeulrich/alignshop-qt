# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri May  6 11:18:39 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
QT += sql
TARGET = TestAbstractDbSource
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += AbstractDbSource.h IDbSource.h
SOURCES += TestAbstractDbSource.cpp \
           AbstractDbSource.cpp

DEFINES += TESTING
