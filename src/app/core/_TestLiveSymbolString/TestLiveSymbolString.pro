# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Jun 28 08:53:56 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestLiveSymbolString
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../LiveSymbolString.h \
           ../ObservableMsa.h \
           ../AbstractLiveCharCountDistribution.h \
           ../LiveMsaCharCountDistribution.h

SOURCES += TestLiveSymbolString.cpp \
           ../LiveSymbolString.cpp \
           ../BioSymbol.cpp \
           ../BioSymbolGroup.cpp \
           ../Seq.cpp \
           ../UngappedSubseq.cpp \
           ../Subseq.cpp \
           ../BioString.cpp \
           ../misc.cpp \
           ../constants.cpp \
           ../Msa.cpp \
           ../ObservableMsa.cpp \
           ../CharCountDistribution.cpp \
           ../LiveMsaCharCountDistribution.cpp \
           ../Services/SymbolStringCalculator.cpp \
           ../util/MsaAlgorithms.cpp

DEFINES += TESTING
