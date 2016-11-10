CONFIG += qtestlib debug
TEMPLATE = app
TARGET =
DEPENDPATH += . ../..
INCLUDEPATH += . ../..

# Input
HEADERS += Msa.h
SOURCES += TestMsa.cpp \
           Msa.cpp \
           Subseq.cpp \
           AnonSeq.cpp \
           BioString.cpp \
           AminoString.cpp \
           DnaString.cpp \
           RnaString.cpp \
           global.cpp \
           util/Rect.cpp \
           util/MsaRect.cpp

DEFINES += TESTING

# QMAKE_CXXFLAGS_DEBUG += -O2 -march=nocona
