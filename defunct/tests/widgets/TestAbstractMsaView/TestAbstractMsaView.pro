# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jan 17 10:17:11 2011)
#
# Copyright (C) 2010 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT += gui \
      svg

TEMPLATE = app
TARGET = TestAbstractMsaView
DEPENDPATH += . ../../.. ../../../widgets ../../../util
INCLUDEPATH += . ../../.. ../../../widgets ../../../util

HEADERS += AbstractMsaView.h \
           AbstractCharPixmapProvider.h \
           MockMsaView.h \
           AbstractTextRenderer.h \
           TextImageRenderer.h \
           TextPixmapRenderer.h \
           AbstractCharPixelMetrics.h \
           ExactTextRenderer.h \
           Msa.h
SOURCES += TestAbstractMsaView.cpp \
           AbstractMsaView.cpp \
           MockMsaView.cpp \
           AbstractCharPixmapProvider.cpp \
           PositionalMsaColorProvider.cpp \
           AbstractTextRenderer.cpp \
           AbstractCharPixelMetrics.cpp \
           TextImageRenderer.cpp \
           TextPixmapRenderer.cpp \
           CharPixelMetrics.cpp \
           CharPixelMetricsF.cpp \
           BioString.cpp \
           Msa.cpp \
           Subseq.cpp \
           AnonSeq.cpp \
           global.cpp \
           util/Rect.cpp \
           util/MsaRect.cpp

DEFINES += TESTING
