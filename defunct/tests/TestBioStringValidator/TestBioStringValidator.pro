CONFIG += qtestlib debug
TARGET = TestBioStringValidator
TEMPLATE = app
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += BioStringValidator.h
SOURCES += TestBioStringValidator.cpp \
           BioStringValidator.cpp \
           BioString.cpp \
           global.cpp
