# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jun 27 14:41:24 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestLiveMsaCharCountDistribution
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../LiveMsaCharCountDistribution.h \
           ../AbstractLiveCharCountDistribution.h \
           ../ObservableMsa.h
SOURCES += TestLiveMsaCharCountDistribution.cpp \
           ../LiveMsaCharCountDistribution.cpp \
           ../ObservableMsa.cpp \
           ../Msa.cpp \
           ../UngappedSubseq.cpp \
           ../Subseq.cpp \
           ../BioString.cpp \
           ../Seq.cpp \
           ../misc.cpp \
           ../constants.cpp \
           ../util/MsaAlgorithms.cpp \
           ../util/Rect.cpp \
           ../util/PosiRect.cpp \
           ../CharCountDistribution.cpp

DEFINES += TESTING
