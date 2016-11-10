# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Wed Dec 21 17:19:55 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestClustalMsaExporter
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../ClustalMsaExporter.h \
    ../../../Entities/AbstractBasicEntity.h \
    ../../../Entities/AbstractMsa.h \
    ../../../Entities/AbstractSeq.h \
    ../../../constants.h \
    ../../../misc.h \
    ../../../UngappedSubseq.h \
    ../../../Subseq.h \
    ../../../BioString.h \
    ../../../Entities/AminoMsa.h \
    ../../../Entities/AminoSeq.h \
    ../../../Msa.h \
    ../../../ObservableMsa.h \
    ../../../Seq.h \
    ../../../Entities/Astring.h

SOURCES += TestClustalMsaExporter.cpp \
           ../ClustalMsaExporter.cpp \
    ../../../Entities/AbstractBasicEntity.cpp \
    ../../../Entities/AbstractMsa.cpp \
    ../../../Entities/AbstractSeq.cpp \
    ../../../constants.cpp \
    ../../../misc.cpp \
    ../../../UngappedSubseq.cpp \
    ../../../Subseq.cpp \
    ../../../BioString.cpp \
    ../../../Entities/AminoMsa.cpp \
    ../../../Entities/AminoSeq.cpp \
    ../../../Msa.cpp \
    ../../../ObservableMsa.cpp \
    ../../../Seq.cpp \
    ../../../Entities/Astring.cpp

DEFINES += TESTING
