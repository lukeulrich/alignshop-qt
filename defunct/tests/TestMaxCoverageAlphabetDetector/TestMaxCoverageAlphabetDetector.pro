CONFIG += qtestlib debug
TEMPLATE = app
TARGET = TestMaxCoverageAlphabetDetector
DEPENDPATH += . ../..
INCLUDEPATH += . ../..
SOURCES += TestMaxCoverageAlphabetDetector.cpp \
           MaxCoverageAlphabetDetector.cpp \
           AbstractAlphabetDetector.cpp \
           BioString.cpp \
           BioStringValidator.cpp \
           AlphabetInspector.cpp \
           global.cpp
