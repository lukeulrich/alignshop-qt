QT += xml xmlpatterns sql
CONFIG += qtestlib debug
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../.. ../../models
INCLUDEPATH += . ../.. ../../models


# Input
HEADERS += Adoc.h AdocTreeModel.h
SOURCES += TestAdoc.cpp Adoc.cpp AdocTreeModel.cpp AdocDbDataSource.cpp DbDataSource.cpp DataRow.cpp DbSpec.cpp AdocTreeNode.cpp CrudSqlRecord.cpp DbTable.cpp MpttTreeConverter.cpp DbAnonSeqFactory.cpp BioString.cpp AnonSeq.cpp AnonSeqFactory.cpp
LIBS += -lsqlite3
