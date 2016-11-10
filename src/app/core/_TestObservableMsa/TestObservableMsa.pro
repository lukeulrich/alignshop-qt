# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jun 23 11:11:33 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestObservableMsa
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../ObservableMsa.h
SOURCES += TestObservableMsa.cpp \
           ../ObservableMsa.cpp \
           ../Msa.cpp \
           ../BioString.cpp \
           ../UngappedSubseq.cpp \
           ../Subseq.cpp \
           ../Seq.cpp \
           ../constants.cpp \
           ../misc.cpp

DEFINES += TESTING
