QT -= gui
CONFIG += qtestlib debug
TEMPLATE = app

INCLUDEPATH += . ../..
DEPENDPATH += . ../..

SOURCES += TestCharCountDistribution.cpp CharCountDistribution.cpp
