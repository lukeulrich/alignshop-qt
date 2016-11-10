QT -= gui
CONFIG += qtestlib debug
TEMPLATE = app
TARGET = TestDataFormatDetector
DEPENDPATH += . ../..
INCLUDEPATH += . ../..
SOURCES += TestDataFormatDetector.cpp DataFormatDetector.cpp DataFormat.cpp FastaFormatInspector.cpp ClustalFormatInspector.cpp
