CONFIG += qtestlib debug
TEMPLATE = app

INCLUDEPATH += . ../..
DEPENDPATH += . ../..

HEADERS += Msa.h
SOURCES += TestMsaAlgorithms.cpp MsaAlgorithms.cpp BioString.cpp global.cpp Msa.cpp Subseq.cpp AnonSeq.cpp
