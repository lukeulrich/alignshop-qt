# -------------------------------------------------
# Project created by QtCreator 2009-11-02T09:55:13
# -------------------------------------------------
# To utilize fast string concatenation with % instead of +
# See: http://doc.trolltech.com/4.6/qstring.html#more-efficient-string-construction
# DEFINES += QT_USE_FAST_CONCATENATION
# Update - 25 August 2010: Elected to utilize #include <QtCore/QStringBuilder> for fast
# concatenation on a per-file basis rather than a global define.
# INCLUDEPATH += /usr/include/freetype2

OBJECTS_DIR = objs
MOC_DIR = auto_moc
TARGET = AlignShop
TEMPLATE = app
SOURCES += main.cpp \
    MainWindow.cpp \
    SerialGenerator.cpp \
    BioString.cpp \
    AminoString.cpp \
    DnaString.cpp \
    RnaString.cpp \
    AnonSeq.cpp \
    Subseq.cpp \
    Msa.cpp \
    FastaParser.cpp \
    ClustalParser.cpp \
    AdocTreeNode.cpp \
    DbSpec.cpp \
    CrudSqlRecord.cpp \
    DbTable.cpp \
    models/AdocTreeModel.cpp \
    widgets/AdocDataTreeView.cpp \
    AnonSeqFactory.cpp \
    DbAnonSeqFactory.cpp \
    models/AdocTypeFilterModel.cpp \
    DataFormat.cpp \
    BioStringValidator.cpp \
    DataFormatDetector.cpp \
    ThermodynamicCalculator.cpp \
    AlphabetInspector.cpp \
    RebaseParser.cpp \
    AbstractAlphabetDetector.cpp \
    MaxCoverageAlphabetDetector.cpp \
    FastaFormatInspector.cpp \
    ClustalFormatInspector.cpp \
    DnaSecondaryStructure.cpp \
    forms/ImportSequencesDialog.cpp \
    models/ParsedBioStringTableModel.cpp \
    forms/ProjectGroupSelectionDialog.cpp \
    widgets/LineEditDelegate.cpp \
    forms/PrimerFinderDialog.cpp \
    models/RestrictionEnzymeTableModel.cpp \
    DataRow.cpp \
    DbDataSource.cpp \
    MpttTreeConverter.cpp \
    utility.cpp \
    global.cpp \
    forms/RestrictionEnzymeDialog.cpp \
    DbRowCacheManager.cpp \
    models/AminoSliceModel.cpp \
    models/DnaSliceModel.cpp \
    models/RnaSliceModel.cpp \
    models/AllSliceModel.cpp \
    models/SliceSortProxyModel.cpp \
    models/AdocSortFilterProxyModel.cpp \
    MpttNode.cpp \
    models/TableModel.cpp \
    models/RelatedTableModel.cpp \
    TagGenerator.cpp \
    models/SliceProxyModel.cpp \
    models/SubseqSliceModel.cpp \
    SqlBuilder.cpp \
    SynchronousAdocDataSource.cpp \
    ThreadedAdocDataSource.cpp \
    models/MsaSubseqModel.cpp \
    forms/MsaWindow.cpp \
    widgets/AbstractMsaView.cpp \
    MsaAlgorithms.cpp \
    PositionalMsaColorProvider.cpp \
    LiveMsaCharCountDistribution.cpp \
    CharCountDistribution.cpp \
    BioSymbol.cpp \
    BioSymbolGroup.cpp \
    LiveSymbolString.cpp \
    CharColorScheme.cpp \
    BioSymbolColorScheme.cpp \
    forms/PrimerDesign/SequenceView.cpp \
    forms/PrimerDesign/SequenceDetailsPanel.cpp \
    forms/PrimerDesign/PrimerResultsPage.cpp \
    forms/PrimerDesign/PrimerListPanel.cpp \
    forms/PrimerDesign/PrimerDesignWizard.cpp \
    forms/PrimerDesign/PrimerDesignBasePage.cpp \
    forms/PrimerDesign/NewSequenceDialog.cpp \
    forms/PrimerDesign/Labels.cpp \
    forms/PrimerDesign/PrimerDesignInputPage.cpp \
    PrimerDesign/StrictDnaString.cpp \
    PrimerDesign/PrimerPairListModel.cpp \
    PrimerDesign/PrimerPair.cpp \
    PrimerDesign/PrimerDesignFile.cpp \
    PrimerDesign/Primer.cpp \
    PrimerDesign/ObservableSequence.cpp \
    PrimerDesign/IPrimerDesignOM.cpp \
    PrimerDesign/DnaSequence.cpp \
    PrimerDesign/AmpliconRange.cpp \
    ObservableListNotifier.cpp \
    PrimerDesign/SequenceListModel.cpp \
    PrimerDesign/ListModel.cpp \
    AbstractCharPixmapProvider.cpp \
    FontCharPixmapProvider.cpp \
    SymbolColorScheme.cpp \
    SymbolColorProvider.cpp \
    forms/PrimerDesign/PrimerGenerationDialog.cpp \
    forms/PrimerDesign/PrimerParamsPreviewDialog.cpp \
    widgets/SequenceTextView.cpp \
    forms/PrimerDesign/RestrictionEnzymeTextbox.cpp \
    PrimerDesign/PrimerDesignInput.cpp \
    SequenceValidator.cpp \
    widgets/NativeMsaView.cpp \
    util/Rect.cpp \
    TextImageRenderer.cpp \
    TextPixmapRenderer.cpp \
    PrimerDesign/PrimerPairNamer.cpp \
    util/CharPixelMetrics.cpp \
    AbstractTextRenderer.cpp \
    util/CharPixelMetricsF.cpp \
    util/AbstractCharPixelMetrics.cpp \
    util/MsaRect.cpp \
    forms/PrimerDesign/PrimerPairHighlighter.cpp \
    forms/PrimerDesign/ThreePrimeInput.cpp \
    PrimerDesign/PrimerPairFinder.cpp \
    PrimerDesign/DimerCalculator.cpp
HEADERS += MainWindow.h \
    SerialGenerator.h \
    BioString.h \
    AminoString.h \
    DnaString.h \
    RnaString.h \
    AnonSeq.h \
    Subseq.h \
    Msa.h \
    SimpleSeq.h \
    SimpleSeqParser.h \
    FastaParser.h \
    ClustalParser.h \
    GenError.h \
    ParseError.h \
    TreeNode.h \
    AdocTreeNode.h \
    DbSpec.h \
    CrudSqlRecord.h \
    DbTable.h \
    models/AdocTreeModel.h \
    widgets/AdocDataTreeView.h \
    AnonSeqFactory.h \
    DbAnonSeqFactory.h \
    models/AdocTypeFilterModel.h \
    RebaseParser.h \
    DataFormat.h \
    BioStringValidator.h \
    global.h \
    DataFormatDetector.h \
    AbstractDataFormatInspector.h \
    FastaFormatInspector.h \
    ClustalFormatInspector.h \
    SingletonPtr.h \
    ThermodynamicCalculator.h \
    AlphabetInspector.h \
    BioStringValidator.h \
    Picker.h \
    AbstractAlphabetDetector.h \
    MaxCoverageAlphabetDetector.h \
    ClustalFormatInspector.h \
    DnaSecondaryStructure.h \
    forms/ImportSequencesDialog.h \
    models/ParsedBioStringTableModel.h \
    forms/ProjectGroupSelectionDialog.h \
    widgets/LineEditDelegate.h \
    forms/PrimerFinderDialog.h \
    models/RestrictionEnzymeTableModel.h \
    ParsedBioString.h \
    DataRow.h \
    DbDataSource.h \
    exceptions/Exception.h \
    exceptions/RuntimeError.h \
    exceptions/DatabaseError.h \
    exceptions/InvalidConnectionError.h \
    MpttNode.h \
    MpttTreeConverter.h \
    exceptions/InvalidMpttNodeError.h \
    exceptions/CodedError.h \
    exceptions/FatalError.h \
    utility.h \
    forms/RestrictionEnzymeDialog.h \
    DbRowCacheManager.h \
    models/AminoSliceModel.h \
    models/DnaSliceModel.h \
    models/RnaSliceModel.h \
    models/AllSliceModel.h \
    models/SliceSortProxyModel.h \
    models/AdocSortFilterProxyModel.h \
    models/TableModel.h \
    models/RelatedTableModel.h \
    TagGenerator.h \
    models/SliceProxyModel.h \
    models/SubseqSliceModel.h \
    SqlBuilder.h \
    AbstractAdocDataSource.h \
    SynchronousAdocDataSource.h \
    ThreadedAdocDataSource.h \
    models/MsaSubseqModel.h \
    forms/MsaWindow.h \
    widgets/AbstractMsaView.h \
    MsaAlgorithms.h \
    PositionalMsaColorProvider.h \
    TextColorStyle.h \
    LiveMsaCharCountDistribution.h \
    CharCountDistribution.h \
    LiveCharCountDistribution.h \
    BioSymbol.h \
    BioSymbolGroup.h \
    LiveSymbolString.h \
    CharColorScheme.h \
    BioSymbolColorScheme.h \
    forms/PrimerDesign/SequenceView.h \
    forms/PrimerDesign/SequenceDetailsPanel.h \
    forms/PrimerDesign/PrimerResultsPage.h \
    forms/PrimerDesign/PrimerListPanel.h \
    forms/PrimerDesign/PrimerDesignWizard.h \
    forms/PrimerDesign/PrimerDesignBasePage.h \
    forms/PrimerDesign/NewSequenceDialog.h \
    forms/PrimerDesign/Labels.h \
    PrimerDesign/StrictDnaString.h \
    PrimerDesign/Range.h \
    PrimerDesign/PrimerPairListModel.h \
    PrimerDesign/PrimerPairGroup.h \
    PrimerDesign/PrimerPair.h \
    PrimerDesign/PrimerDesignInput.h \
    PrimerDesign/PrimerDesignFile.h \
    PrimerDesign/Primer.h \
    PrimerDesign/ObservableSequence.h \
    PrimerDesign/IPrimerDesignOM.h \
    PrimerDesign/DnaSequence.h \
    PrimerDesign/AmpliconRange.h \
    ObservableListNotifier.h \
    ObservableList.h \
    PrimerDesign/SequenceListModel.h \
    PrimerDesign/ListModel.h \
    AbstractCharPixmapProvider.h \
    FontCharPixmapProvider.h \
    SymbolColorScheme.h \
    SymbolColorProvider.h \
    forms/PrimerDesign/PrimerGenerationDialog.h \
    forms/PrimerDesign/PrimerParamsPreviewDialog.h \
    widgets/SequenceTextView.h \
    forms/PrimerDesign/RestrictionEnzymeTextbox.h \
    SequenceValidator.h \
    widgets/NativeMsaView.h \
    util/Rect.h \
    TextImageRenderer.h \
    TextPixmapRenderer.h \
    widgets/PercentSpinBox.h \
    widgets/AgDoubleSpinBox.h \
    PrimerDesign/PrimerPairNamer.h \
    util/AbstractCharPixelMetrics.h \
    AbstractTextRenderer.h \
    util/CharPixelMetrics.h \
    util/CharPixelMetricsF.h \
    ExactTextRenderer.h \
    BasicTextRenderer.h \
    util/MsaRect.h \
    forms/PrimerDesign/PrimerPairHighlighter.h \
    forms/PrimerDesign/ThreePrimeInput.h \
    PrimerDesign/PrimerPairFinder.h \
    PrimerDesign/DimerCalculator.h \
    forms/PrimerDesign/PrimerDesignInputPage.h
FORMS += forms/MainWindow.ui \
    forms/ImportSequencesDialog.ui \
    forms/PrimerFinderDialog.ui \
    forms/ProjectGroupSelectionDialog.ui \
    forms/RestrictionEnzymeDialog.ui \
    forms/MsaWindow.ui \
    forms/PrimerDesign/SequenceView.ui \
    forms/PrimerDesign/SequenceDetailsPanel.ui \
    forms/PrimerDesign/PrimerResultsPage.ui \
    forms/PrimerDesign/PrimerListPanel.ui \
    forms/PrimerDesign/PrimerDesignWizard.ui \
    forms/PrimerDesign/NewSequenceDialog.ui \
    forms/PrimerDesign/PrimerDesignInputPage.ui \
    forms/PrimerDesign/PrimerGenerationDialog.ui \
    forms/PrimerDesign/PrimerParamsPreviewDialog.ui \
    forms/PrimerDesign/ThreePrimeInput.ui
RESOURCES += resources/AlignShop.qrc
OTHER_FILES += log.txt \
    build_notes.txt \
    schemas/db_schema-0.1.xsd \
    perl/sql2xml.pl \
    perl/cpp_skeleton.pl \
    db_specs/db_spec-0.1.xml \
    db_specs/ddl-0.2.sql \
    bugs.txt \
    Conference_Questions.txt \
    outstanding_issues.txt \
    Conference_notes.txt \
    UserStories.txt \
    resources/AlignShop.rc \
    conventions.txt
QT += sql \
    xml \
    xmlpatterns \
    svg
debug {
    include(models/modeltest/modeltest.pri)

    # On Unix, the following flags are needed to incorporate profiling support
    unix {
#        QMAKE_CXXFLAGS += -pg
#        QMAKE_LFLAGS += -pg
#        QMAKE_CXXFLAGS_RELEASE -= -fomit-frame-pointer
#        QMAKE_CFLAGS_RELEASE -= -fomit-frame-pointer
    }
}
win32 {
    # This line informs windows compiler with MSVC to run uic to generate the necessary form include files
    CONFIG += uic
    RC_FILE = resources/AlignShop.rc

    # This line enables link-time optimization which results in ~10-15%
    release:QMAKE_LFLAGS_RELEASE = /LTCG /O2
    release:QMAKE_LIBFLAGS_RELEASE = /LTCG /O2
}
