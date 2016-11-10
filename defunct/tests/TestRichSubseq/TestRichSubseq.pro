CONFIG += qtestlib debug
QT -= gui
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

# Input
SOURCES += TestRichSubseq.cpp RichSubseq.cpp Subseq.cpp AnonSeq.cpp BioString.cpp DataRow.cpp global.cpp
