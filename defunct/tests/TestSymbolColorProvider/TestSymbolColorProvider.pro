# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jan 31 16:54:51 2011)
#
# Copyright (C) 111  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestSymbolColorProvider
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += PositionalMsaColorProvider.h \
           LiveCharCountDistribution.h \
           LiveMsaCharCountDistribution.h \
           CharColorScheme.h \
           Msa.h \
           LiveSymbolString.h
SOURCES += TestSymbolColorProvider.cpp \
           SymbolColorProvider.cpp \
           PositionalMsaColorProvider.cpp \
           LiveSymbolString.cpp \
           LiveMsaCharCountDistribution.cpp \
           CharCountDistribution.cpp \
           BioString.cpp \
           BioSymbolGroup.cpp \
           BioSymbol.cpp \
           Msa.cpp \
           AnonSeq.cpp \
           Subseq.cpp \
           SymbolColorScheme.cpp \
           CharColorScheme.cpp \
           global.cpp \
           MsaAlgorithms.cpp

DEFINES += TESTING
