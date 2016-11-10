CONFIG += qtestlib debug
TARGET = TestAlphabetInspector
TEMPLATE = app
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += AlphabetInspector.h
SOURCES += TestAlphabetInspector.cpp \
           AlphabetInspector.cpp \
           BioStringValidator.cpp \
           BioString.cpp \
           global.cpp

