# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue May 24 16:48:32 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestClustalParser
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += ClustalParser.h \
           ISequenceParser.h
SOURCES += TestClustalParser.cpp \
           ClustalParser.cpp \
           ../AbstractSequenceParser.cpp \
           ../BioString.cpp \
           ../misc.cpp \
           ../constants.cpp

DEFINES += TESTING
