# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue May 31 11:12:33 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestDataFormatDetector
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../DataFormatDetector.h \
           ../IDataFormatDetector.h \
           ../../Parsers/ISequenceParser.h \
           ../../Parsers/FastaParser.h \
           ../../Parsers/ClustalParser.h

SOURCES += TestDataFormatDetector.cpp \
           ../DataFormatDetector.cpp \
           ../../DataFormat.cpp \
           ../../Parsers/AbstractSequenceParser.cpp \
           ../../Parsers/FastaParser.cpp \
           ../../Parsers/ClustalParser.cpp \
           ../../BioString.cpp \
           ../../constants.cpp \
           ../../misc.cpp

DEFINES += TESTING
