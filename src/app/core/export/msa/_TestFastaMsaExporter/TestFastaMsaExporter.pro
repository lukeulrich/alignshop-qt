# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Dec 20 18:49:53 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestFastaMsaExporter
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../FastaMsaExporter.h \
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
SOURCES += TestFastaMsaExporter.cpp \
           ../FastaMsaExporter.cpp \
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














