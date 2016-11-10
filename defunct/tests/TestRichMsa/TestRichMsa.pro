CONFIG += qtestlib debug
QT -= gui
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

# Input
HEADERS += Msa.h RichMsa.h
SOURCES += TestRichMsa.cpp \
           Msa.cpp \
           Subseq.cpp \
           AnonSeq.cpp \
           BioString.cpp \
           AminoString.cpp \
           DnaString.cpp \
           DataRow.cpp \
           global.cpp
