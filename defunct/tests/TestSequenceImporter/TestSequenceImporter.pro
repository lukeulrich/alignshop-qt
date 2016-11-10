QT += sql
CONFIG += qtestlib debug
TEMPLATE = app

INCLUDEPATH += . ../.. ../../models
DEPENDPATH += . ../.. ../../models

HEADERS += models/AdocTreeModel.h
SOURCES += TestSequenceImporter.cpp SequenceImporter.cpp AdocTreeNode.cpp DbAnonSeqFactory.cpp AnonSeqFactory.cpp BioString.cpp AminoString.cpp AnonSeq.cpp models/AdocTreeModel.cpp DbDataSource.cpp AdocDbDataSource.cpp DataRow.cpp global.cpp
