CONFIG += qtestlib debug
TARGET = TestFastaFormatInspector
TEMPLATE = app
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += FastaFormatInspector.h
SOURCES += TestFastaFormatInspector.cpp FastaFormatInspector.cpp DataFormat.cpp
