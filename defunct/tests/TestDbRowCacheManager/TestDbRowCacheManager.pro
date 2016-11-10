QT += sql
CONFIG += qtestlib debug
TEMPLATE = app
TARGET =
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

SOURCES += TestDbRowCacheManager.cpp DbRowCacheManager.cpp DbDataSource.cpp DataRow.cpp global.cpp

DEFINES += TESTING
