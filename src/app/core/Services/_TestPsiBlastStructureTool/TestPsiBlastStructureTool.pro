# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Mon Aug 29 09:27:02 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPsiBlastStructureTool
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../PsiBlastStructureTool.h \
           ../PsiBlastWrapper.h
SOURCES += TestPsiBlastStructureTool.cpp \
           ../PsiBlastStructureTool.cpp \
           ../../BioString.cpp \
           ../../PODs/PsiBlastConfig.cpp \
           ../PsiBlastWrapper.cpp \
           ../FannWrapper.cpp \
           ../NNStructureTool.cpp \
           ../../Parsers/PssmParser.cpp \
           ../../util/PssmUtil.cpp \
           ../../misc.cpp \
           ../../constants.cpp

unix {
    LIBS += -L../../../../fann-2.1.0/src/.libs -lfann
}

DEFINES += TESTING
