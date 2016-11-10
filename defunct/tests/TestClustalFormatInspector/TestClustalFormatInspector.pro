CONFIG += qtestlib debug
TARGET = TestClustalFormatInspector
TEMPLATE = app
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

HEADERS += ClustalFormatInspector.h
SOURCES += TestClustalFormatInspector.cpp ClustalFormatInspector.cpp DataFormat.cpp
