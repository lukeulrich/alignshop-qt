QT += sql xmlpatterns
CONFIG += qtestlib debug
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models

# Input
HEADERS += AdocTreeModel.h AdocTypeFilterModel.h
SOURCES += TestAdocTypeFilterModel.cpp AdocTypeFilterModel.cpp AdocTreeModel.cpp AdocTreeNode.cpp CrudSqlRecord.cpp DbTable.cpp
