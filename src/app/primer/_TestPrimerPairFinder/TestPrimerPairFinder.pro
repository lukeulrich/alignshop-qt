# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu Jan 12 22:51:02 2012)
#
# Copyright (C) 2012  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib
QT -= gui
TARGET = TestPrimerPairFinder
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../PrimerPairFinder.h
SOURCES += TestPrimerPairFinder.cpp \
           ../PrimerPairFinder.cpp \
           ../PrimerPairFactory.cpp \
           ../PrimerFactory.cpp \
           ../Primer.cpp \
           ../PrimerSearchParameters.cpp \
           ../PrimerPair.cpp \
           ../ThermodynamicCalculator.cpp \
           ../ThermodynamicConstants.cpp \
           ../DimerScoreCalculator.cpp \
           ../../core/util/ClosedIntRange.cpp \
           ../../core/BioString.cpp \
           ../../core/DnaPattern.cpp \
           ../../core/misc.cpp \
           ../../core/constants.cpp

DEFINES += TESTING


debug_and_release {
    CONFIG -= debug_and_release
}
CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

release {
    DEFINES += QT_NO_DEBUG
    DEFINES += QT_NO_DEBUG_OUTPUT
    QMAKE_CXXFLAGS_RELEASE += -O2
}
