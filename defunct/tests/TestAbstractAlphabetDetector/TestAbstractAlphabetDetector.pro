CONFIG += qtestlib debug
TARGET = TestAbstractAlphabetDetector
TEMPLATE = app
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += AbstractAlphabetDetector.h
SOURCES += TestAbstractAlphabetDetector.cpp \
           AbstractAlphabetDetector.cpp \
           AlphabetInspector.cpp \
           BioString.cpp \
           BioStringValidator.cpp \
           global.cpp
