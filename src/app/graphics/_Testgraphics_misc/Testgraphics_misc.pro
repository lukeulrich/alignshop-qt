# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Tue Jun 28 08:38:50 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
TARGET = Testgraphics_misc
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../graphics_misc.h
SOURCES += Testgraphics_misc.cpp \
           ../graphics_misc.cpp

DEFINES += TESTING
