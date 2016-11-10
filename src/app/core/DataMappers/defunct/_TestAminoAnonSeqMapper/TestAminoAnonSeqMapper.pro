# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue May  3 10:40:55 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestAminoAnonSeqMapper
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += AminoAnonSeqMapper.h
SOURCES += TestAminoAnonSeqMapper.cpp \
           AminoAnonSeqMapper.cpp \
           ../../BioString.cpp \
           ../../misc.cpp \
           ../../Seq.cpp \
           ../../constants.cpp \
           ../../Entities/AminoAnonSeq.cpp

DEFINES += TESTING
