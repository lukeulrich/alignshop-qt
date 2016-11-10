# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Jun 28 15:21:20 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestSymbolColorProvider
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../SymbolColorProvider.h \
           ../../core/ObservableMsa.h \
           ../../core/LiveMsaCharCountDistribution.h \
           ../../core/LiveSymbolString.h \
           ../../core/AbstractLiveCharCountDistribution.h
SOURCES += TestSymbolColorProvider.cpp \
           ../SymbolColorProvider.cpp \
           ../SymbolColorScheme.cpp \
           ../CharColorScheme.cpp \
           ../PositionalMsaColorProvider.cpp \
           ../../core/BioString.cpp \
           ../../core/BioSymbol.cpp \
           ../../core/BioSymbolGroup.cpp \
           ../../core/Seq.cpp \
           ../../core/Subseq.cpp \
           ../../core/UngappedSubseq.cpp \
           ../../core/constants.cpp \
           ../../core/misc.cpp \
           ../../core/Msa.cpp \
           ../../core/ObservableMsa.cpp \
           ../../core/CharCountDistribution.cpp \
           ../../core/LiveMsaCharCountDistribution.cpp \
           ../../core/LiveSymbolString.cpp \
           ../../core/Services/SymbolStringCalculator.cpp \
           ../../core/util/MsaAlgorithms.cpp


DEFINES += TESTING
