# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Fri Aug 26 11:44:06 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestPsiBlastWrapper
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../PsiBlastWrapper.h \
           ../AbstractProcessWrapper.h
SOURCES += TestPsiBlastWrapper.cpp \
           ../AbstractProcessWrapper.cpp \
           ../PsiBlastWrapper.cpp \
           ../../BioString.cpp \
           ../../constants.cpp \
           ../../misc.cpp \
           ../../PODs/PsiBlastConfig.cpp

DEFINES += TESTING
