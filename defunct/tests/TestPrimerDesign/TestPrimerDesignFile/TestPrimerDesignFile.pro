
CONFIG += qtestlib debug
TEMPLATE = app
TARGET =
DEPENDPATH += . ../../.. ../../../PrimerDesign
INCLUDEPATH += . ../../.. ../../../PrimerDesign
QT += sql \
    xml \
    xmlpatterns

SOURCES += \
    TestPrimerDesignFile.cpp \
    ../../../ObservableListNotifier.cpp \
    ../../../PrimerDesign/StrictDnaString.cpp \
    ../../../PrimerDesign/SequenceListModel.cpp \
    ../../../PrimerDesign/PrimerSet.cpp \
    ../../../PrimerDesign/PrimerPairListModel.cpp \
    ../../../PrimerDesign/PrimerPairGroup.cpp \
    ../../../PrimerDesign/PrimerPair.cpp \
    ../../../PrimerDesign/PrimerDesigninput_.cpp \
    ../../../PrimerDesign/PrimerDesignFile.cpp \
    ../../../PrimerDesign/Primer.cpp \
    ../../../PrimerDesign/ParameterSet.cpp \
    ../../../PrimerDesign/ObservableSequence.cpp \
    ../../../PrimerDesign/ListModel.cpp \
    ../../../PrimerDesign/IPrimerDesignOM.cpp \
    ../../../PrimerDesign/DnaSequence.cpp \
    ../../../PrimerDesign/AmpliconRange.cpp \
    ../../../BioStringValidator.cpp \
    ../../../BioString.cpp \
    ../../../global.cpp

HEADERS += \
    ../../../ObservableListNotifier.h \
    ../../../ObservableList.h \
    ../../../PrimerDesign/StrictDnaString.h \
    ../../../PrimerDesign/SequenceListModel.h \
    ../../../PrimerDesign/Range.h \
    ../../../PrimerDesign/PrimerSet.h \
    ../../../PrimerDesign/PrimerPairListModel.h \
    ../../../PrimerDesign/PrimerPairGroup.h \
    ../../../PrimerDesign/PrimerPair.h \
    ../../../PrimerDesign/PrimerDesigninput_.h \
    ../../../PrimerDesign/PrimerDesignFile.h \
    ../../../PrimerDesign/Primer.h \
    ../../../PrimerDesign/parameterset.h \
    ../../../PrimerDesign/ObservableSequence.h \
    ../../../PrimerDesign/ListModel.h \
    ../../../PrimerDesign/IPrimerDesignOM.h \
    ../../../PrimerDesign/DnaSequence.h \
    ../../../PrimerDesign/AmpliconRange.h \
    ../../../BioStringValidator.h \
    ../../../BioString.h \
    ../../../global.h

OTHER_FILES += \
    TestData.xml

RESOURCES += \
    TestData.qrc
