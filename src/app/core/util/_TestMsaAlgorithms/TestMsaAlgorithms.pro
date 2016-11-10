# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jun 27 13:47:51 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestMsaAlgorithms
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../MsaAlgorithms.h
SOURCES += TestMsaAlgorithms.cpp \
           ../MsaAlgorithms.cpp \
           ../../Msa.cpp \
           ../../Seq.cpp \
           ../../UngappedSubseq.cpp \
           ../../BioString.cpp \
           ../../constants.cpp \
           ../../misc.cpp \
           ../../Subseq.cpp \
           ../../CharCountDistribution.cpp

DEFINES += TESTING
