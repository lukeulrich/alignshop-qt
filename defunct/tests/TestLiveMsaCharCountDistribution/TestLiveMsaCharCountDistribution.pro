CONFIG += qtestlib debug
TEMPLATE = app

INCLUDEPATH += . ../..
DEPENDPATH += . ../..

HEADERS += LiveMsaCharCountDistribution.h \
  LiveCharCountDistribution.h \
  Msa.h
SOURCES += TestLiveMsaCharCountDistribution.cpp \
  LiveMsaCharCountDistribution.cpp \
  AnonSeq.cpp \
  BioString.cpp \
  CharCountDistribution.cpp \
  Msa.cpp \
  MsaAlgorithms.cpp \
  Subseq.cpp \
  global.cpp

DEFINES += TESTING
