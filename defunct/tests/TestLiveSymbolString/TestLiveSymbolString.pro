# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Dec 17 10:29:47 2010)
#
# Copyright (C) 2010 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = TestLiveSymbolString
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += LiveCharCountDistribution.h \
           Msa.h \
           LiveSymbolString.h \
           LiveMsaCharCountDistribution.h
SOURCES += TestLiveSymbolString.cpp \
           LiveSymbolString.cpp \
           LiveMsaCharCountDistribution.cpp \
           Msa.cpp \
           AnonSeq.cpp \
           BioString.cpp \
           global.cpp \
           Subseq.cpp \
           MsaAlgorithms.cpp \
           CharCountDistribution.cpp \
           BioSymbolGroup.cpp \
           BioSymbol.cpp

DEFINES += TESTING
