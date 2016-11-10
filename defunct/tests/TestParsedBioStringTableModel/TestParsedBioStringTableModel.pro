CONFIG += qtestlib debug
TEMPLATE = app
TARGET = TestParsedBioStringTableModel
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

HEADERS += ParsedBioStringTableModel.h
SOURCES += TestParsedBioStringTableModel.cpp \
           models/ParsedBioStringTableModel.cpp \
           BioString.cpp \
           global.cpp
