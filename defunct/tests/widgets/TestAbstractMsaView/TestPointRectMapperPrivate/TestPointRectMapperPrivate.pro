# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Jan 17 10:17:11 2011)
#
# Copyright (C) 2011 Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT += gui svg
TEMPLATE = app
TARGET = TestPointRectMapperPrivate
DEPENDPATH += . .. ../../../.. ../../../../widgets ../../../../util
INCLUDEPATH += . .. ../../../.. ../../../../widgets ../../../../util

HEADERS += AbstractMsaView.h \
           MockMsaView.h \
           AbstractTextRenderer.h \
           AbstractCharPixelMetrics.h \
           TextImageRenderer.h \
           TextPixmapRenderer.h \
           ExactTextRenderer.h \
           CharPixelMetrics.h \
           CharPixelMetricsF.h \
           Msa.h
SOURCES += TestPointRectMapperPrivate.cpp \
           AbstractMsaView.cpp \
           TextImageRenderer.cpp \
           TextPixmapRenderer.cpp \
           PositionalMsaColorProvider.cpp \
           AbstractTextRenderer.cpp \
           AbstractCharPixelMetrics.cpp \
           CharPixelMetrics.cpp \
           CharPixelMetricsF.cpp \
           MockMsaView.cpp \
           BioString.cpp \
           Msa.cpp \
           Subseq.cpp \
           AnonSeq.cpp \
           global.cpp \
           util/Rect.cpp

DEFINES += TESTING
