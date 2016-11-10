# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri May  6 11:48:26 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
QT += sql
TARGET = TestDbAstringCrud
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += DbAstringCrud.h ../../AbstractDbSource.h
SOURCES += TestDbAstringCrud.cpp \
           DbAstringCrud.cpp \
           ../../AbstractDbSource.cpp \
           ../../../Seq.cpp \
           ../../../BioString.cpp \
           ../../../misc.cpp \
           ../../../constants.cpp \
           ../../Entities/Astring.cpp

DEFINES += TESTING
