# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue May  3 17:00:33 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestAminoAnonSeqRepository
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += AminoAnonSeqRepository.h
SOURCES += TestAminoAnonSeqRepository.cpp \
           AminoAnonSeqRepository.cpp \
           ../../BioString.cpp \
           ../../DataMappers/AminoAnonSeqMapper.cpp \
           ../../Entities/AminoAnonSeq.cpp \
           ../../misc.cpp \
           ../../Seq.cpp \
           ../../constants.cpp

DEFINES += TESTING
