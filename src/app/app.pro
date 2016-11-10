include(../common.pri)

QT       += core gui sql network xml

TARGET = ../AlignShop
TEMPLATE = app

debug {
    DEFINES += MODEL_DATA_CAN_RETURN_QVARIANT
    include(external/modeltest/modeltest.pri)
}

release {
    DEFINES += QT_NO_DEBUG
    DEFINES += QT_NO_DEBUG_OUTPUT
    QMAKE_CXXFLAGS_RELEASE += -O2
}


# Version data
MAJOR_VERSION = 0
MINOR_VERSION = 5
BUILD_TYPE = "a"            # dev -> dev, a -> alpha, b -> beta, rc -> release candidate, r -> stable release
BUILD_NUMBER = 4

VERSION_HEADER = $$PWD/version.h
VERSION_TOOL_EXE_NAME = version-tool
win32:VERSION_TOOL_EXE_NAME = version-tool.exe
versiontarget.target = $$VERSION_HEADER
versiontarget.commands = $$OUT_PWD/../version-tool/$$VERSION_TOOL_EXE_NAME $$MAJOR_VERSION $$MINOR_VERSION $$BUILD_TYPE $$BUILD_NUMBER $$PWD > $$VERSION_HEADER
versiontarget.depends = FORCE
QMAKE_EXTRA_TARGETS = versiontarget
PRE_TARGETDEPS += $$VERSION_HEADER


unix {
    LIBS += -lsqlite3
    LIBS += -L../fann -lfann

    debug {
        # The most relevant warnings below are the shadowing issue and hidden virtual methods
        # -Wshadow -Woverloaded-virtual -Wold-style-cast -Wfloat-equal -Wundef
        QMAKE_CXXFLAGS += -Wextra
    }
    # Uncomment the following to profile the code
    release {
#        QMAKE_CXXFLAGS += -pg
#        QMAKE_LFLAGS += -pg
    }

    # Since, AlignShop is executed with a shell script on linux (to add lib to the LD_LIBRARY_PATH), rename the final
    # AlignShop binary with a .bin extension
    SCRIPT_NAME = $$basename(TARGET)    # AlignShop
    TARGET = ../AlignShop.bin

    # Create the script file using shell commands
    QMAKE_POST_LINK = echo -e $$quote(\\$$LITERAL_HASH!/bin/sh\\\\n\\\\nLD_LIBRARY_PATH=lib ./$$basename(TARGET)) > \
                      ../$$SCRIPT_NAME$$escape_expand(\\n\\t)
    QMAKE_POST_LINK += chmod +x ../$$SCRIPT_NAME
}

mac {
    INCLUDEPATH += ../../../../boost_1_47_0

    # Needed to include zlib support
    LIBS += -lz
}

win32 {
    WIN_QT_SRC_PATH = ../../../$${QT_VERSION}/src

    INCLUDEPATH += $${WIN_QT_SRC_PATH}/3rdparty/sqlite $${WIN_QT_SRC_PATH}/3rdparty/zlib
    SOURCES += $${WIN_QT_SRC_PATH}/3rdparty/sqlite/sqlite3.c

    INCLUDEPATH += ../../../../boost_1_47

    LIBS += -L../fann fann.lib

    # Since all UI files are compiled in UI_DIR, any promoted widgets will not have the proper path.
    # This line adds the root path so that the compiler can find the necessary files (e.g. gui/widgets/AdocTreeView.h)
    INCLUDEPATH += ../../src/app
}


SOURCES += main.cpp\
    core/DataMappers/AminoSeqMapper.cpp \
    core/DataMappers/DnaSeqMapper.cpp \
    core/DataSources/Crud/DbAminoMsaCrud.cpp \
    core/DataSources/Crud/DbAminoSeqCrud.cpp \
    core/DataSources/Crud/DbAstringCrud.cpp \
    core/DataSources/Crud/DbDnaSeqCrud.cpp \
    core/DataSources/Crud/DbDstringCrud.cpp \
    core/DataSources/AbstractDbSource.cpp \
    core/DataSources/SqliteAdocSource.cpp \
    core/Detectors/AlphabetDetector.cpp \
    core/Detectors/DataFormatDetector.cpp \
    core/Entities/AbstractBasicEntity.cpp \
    core/Entities/AbstractMsa.cpp \
    core/Entities/AbstractSeq.cpp \
    core/Entities/AminoMsa.cpp \
    core/Entities/AminoSeq.cpp \
    core/Entities/Astring.cpp \
    core/Entities/DnaSeq.cpp \
    core/Entities/Dstring.cpp \
    core/Parsers/AbstractSequenceParser.cpp \
    core/Parsers/ClustalParser.cpp \
    core/Parsers/FastaParser.cpp \
    core/Services/PodEntityService.cpp \
    core/Services/SymbolStringCalculator.cpp \
    core/util/MsaAlgorithms.cpp \
    core/util/PosiRect.cpp \
    core/util/Rect.cpp \
    core/Adoc.cpp \
    core/BioString.cpp \
    core/BioStringValidator.cpp \
    core/BioSymbol.cpp \
    core/BioSymbolGroup.cpp \
    core/CharCountDistribution.cpp \
    core/DataFormat.cpp \
    core/LiveMsaCharCountDistribution.cpp \
    core/LiveSymbolString.cpp \
    core/Msa.cpp \
    core/ObservableMsa.cpp \
    core/Seq.cpp \
    core/Subseq.cpp \
    core/UngappedSubseq.cpp \
    core/constants.cpp \
    core/misc.cpp \
    graphics/AbstractCharPixelMetrics.cpp \
    graphics/AbstractTextRenderer.cpp \
    graphics/BioSymbolColorScheme.cpp \
    graphics/CharColorScheme.cpp \
    graphics/CharPixelMetrics.cpp \
    graphics/CharPixelMetricsF.cpp \
    graphics/SymbolColorScheme.cpp \
    graphics/SymbolColorProvider.cpp \
    graphics/TextImageRenderer.cpp \
    graphics/TextPixmapRenderer.cpp \
    graphics/graphics_misc.cpp \
    gui/Commands/InsertAdocTreeNodesCommand.cpp \
    gui/Commands/MoveAdocTreeNodesCommand.cpp \
    gui/Commands/RemoveAdocTreeNodesCommand.cpp \
    gui/Commands/SetEntityDataCommand.cpp \
    gui/Services/AdocTreeNodeEraserService.cpp \
    gui/delegates/ClipboardStateItemDelegate.cpp \
    gui/delegates/LineEditDelegate.cpp \
    gui/forms/MainWindow.cpp \
    gui/forms/MsaWindow.cpp \
    gui/forms/SelectGroupNodeDialog.cpp \
    gui/models/AbstractMultiEntityTableModel.cpp \
    gui/models/AdocTreeModel.cpp \
    gui/models/AdocTreeNodeFilterModel.cpp \
    gui/widgets/AbstractMsaView.cpp \
    gui/widgets/AdocTreeView.cpp \
    gui/widgets/MultiSeqTableView.cpp \
    gui/widgets/NativeMsaView.cpp \
    gui/SequenceImporter.cpp \
    gui/gui_misc.cpp \
    gui/Commands/Msa/TrimRowsLeftCommand.cpp \
    gui/Commands/Msa/TrimRowsRightCommand.cpp \
    gui/Commands/Msa/ExtendRowsLeftCommand.cpp \
    gui/Commands/Msa/ExtendRowsRightCommand.cpp \
    gui/Commands/Msa/LevelRowsLeftCommand.cpp \
    gui/Commands/Msa/LevelRowsRightCommand.cpp \
    gui/Commands/Msa/SlideMsaRectCommand.cpp \
    gui/Commands/Msa/InsertGapColumnsCommand.cpp \
    gui/Commands/Msa/RemoveAllGapColumnsCommand.cpp \
    gui/Commands/Msa/CollapseMsaRectLeftCommand.cpp \
    gui/Commands/Msa/CollapseMsaRectRightCommand.cpp \
    gui/models/MsaSubseqTableModel.cpp \
    core/util/QVariantLessGreaterThan.cpp \
    gui/Commands/Msa/SortMsaCommand.cpp \
    graphics/ColorSchemes.cpp \
    gui/models/ColumnAdapters/FilterColumnAdapter.cpp \
    gui/forms/AminoMsaWindow.cpp \
    gui/Commands/Msa/SetSubseqStartCommand.cpp \
    gui/Commands/Msa/SetSubseqStopCommand.cpp \
    gui/Commands/Msa/RemoveRowsCommand.cpp \
    core/machines/events/CustomEventTypes.cpp \
    core/Parsers/SignalSequenceParser.cpp \
    core/metatypes.cpp \
    gui/MsaTools/HandMsaTool.cpp \
    gui/MsaTools/AbstractMsaTool.cpp \
    gui/EventFilters/WidgetFocusObserver.cpp \
    gui/MsaTools/SelectMsaTool.cpp \
    gui/util/PointRectMapper.cpp \
    gui/MsaTools/ZoomMsaTool.cpp \
    gui/MsaTools/GapMsaTool.cpp \
    gui/painting/NativeRenderEngine.cpp \
    graphics/CharColorProvider.cpp \
    gui/Commands/SetGroupLabelCommand.cpp \
    core/Entities/DnaMsa.cpp \
    core/DataSources/Crud/DbDnaMsaCrud.cpp \
    gui/forms/DnaMsaWindow.cpp \
    core/InfoContentDistribution.cpp \
    core/LiveInfoContentDistribution.cpp \
    gui/painting/gitems/LogoBarsItem.cpp \
    core/Services/PsiBlastWrapper.cpp \
    core/Services/PsiBlastStructureTool.cpp \
    core/Services/FannWrapper.cpp \
    core/Services/NNStructureTool.cpp \
    core/Parsers/PssmParser.cpp \
    gui/Services/Tasks/Task.cpp \
    gui/Services/TaskManager.cpp \
    gui/models/TaskModel.cpp \
    gui/Services/Tasks/TaskTreeNode.cpp \
    gui/Services/Tasks/PredictSecondaryTask.cpp \
    graphics/LinearColorScheme.cpp \
    graphics/SecondaryStructureColorProvider.cpp \
    core/util/PssmUtil.cpp \
    graphics/LinearColorStyle.cpp \
    gui/widgets/MsaDataColumnWidget.cpp \
    gui/widgets/AbstractMsaSideWidget.cpp \
    gui/widgets/MsaStartStopSideWidget.cpp \
    gui/widgets/MsaRulerWidget.cpp \
    gui/widgets/SinglePixmapMsaView.cpp \
    gui/painting/gitems/LogoItem.cpp \
    gui/widgets/MsaVertSelectionBar.cpp \
    gui/widgets/MsaTableView.cpp \
    gui/delegates/MsaLineEditDelegate.cpp \
    gui/MsaTools/SelectMsaTool_p.cpp \
    core/Services/AbstractProcessWrapper.cpp \
    core/Services/BlastDatabaseFinder.cpp \
    gui/models/BlastDatabaseModel.cpp \
    gui/forms/BlastDatabaseWindow.cpp \
    core/Services/AbstractBlastDatabaseInstaller.cpp \
    core/Services/NcbiBlastDatabaseInstaller.cpp \
    core/util/Pipe.cpp \
    core/util/GunzipPipe.cpp \
    core/util/GzipPipe.cpp \
    core/util/UntarSink.cpp \
    core/util/TarUtil.cpp \
    gui/forms/BlastDatabaseDownloadWindow.cpp \
    gui/Services/TaskAdocConnector.cpp \
    gui/Commands/InsertBlastReportsCommand.cpp \
    core/Entities/BlastReport.cpp \
    gui/Services/Tasks/BlastTask.cpp \
    gui/forms/BlastDialog.cpp \
    core/DataSources/Crud/DbBlastReportCrud.cpp \
    core/Mptt.tpp \
    core/AdocTreeNode.cpp \
    core/Parsers/xml/BlastXmlHandler.cpp \
    gui/models/BlastReportModel.cpp \
    core/Services/BlastSequenceFetcher.cpp \
    core/Services/BlastDbCmdBase.cpp \
    gui/Services/BlastSequenceImporter.cpp \
    core/Entities/IEntity.cpp \
    core/util/OptionProfile.cpp \
    core/util/OptionSet.cpp \
    core/constants/PsiBlastConstants.cpp \
    core/constants/KalignConstants.cpp \
    core/Services/KalignMsaBuilder.cpp \
    gui/Services/Tasks/BuildMsaTask.cpp \
    gui/Commands/InsertTaskNodesCommand.cpp \
    core/constants/ClustalWConstants.cpp \
    core/Services/ClustalWMsaBuilder.cpp \
    gui/BasicApplication.cpp \
    gui/forms/PredictSecondaryDialog.cpp \
    gui/wizards/MakeBlastDatabaseWizard.cpp \
    core/Services/MakeBlastDatabaseWrapper.cpp \
    core/constants/MakeBlastDbConstants.cpp \
    gui/widgets/FlexTabWidget.cpp \
    gui/forms/flextabs/BlastViewTab.cpp \
    gui/widgets/views/WheelZoomGraphicsView.cpp \
    gui/painting/gitems/AbstractLinearItem.cpp \
    gui/painting/gitems/AbstractRangeItem.cpp \
    gui/painting/gitems/AminoSeqItem.cpp \
    gui/painting/gitems/RangeHandleItem.cpp \
    gui/painting/gitems/BioStringItem.cpp \
    gui/painting/gitems/SeqBioStringItem.cpp \
    gui/painting/gitems/AbstractSeqItem.cpp \
    gui/painting/LinearGraphicsScene.cpp \
    gui/painting/gitems/RangeHandlePairItem.cpp \
    gui/painting/gitems/LinearRulerItem.cpp \
    gui/painting/gitems/DnaSeqItem.cpp \
    gui/models/ColumnAdapters/AminoSeqColumnAdapter.cpp \
    gui/models/ColumnAdapters/DnaSeqColumnAdapter.cpp \
    gui/delegates/SpinBoxDelegate.cpp \
    gui/models/ColumnAdapters/AminoMsaColumnAdapter.cpp \
    gui/models/ColumnAdapters/BlastReportColumnAdapter.cpp \
    gui/models/ColumnAdapters/DnaMsaColumnAdapter.cpp \
    gui/widgets/SequenceTextView.cpp \
    gui/Services/HeaderColumnSelector.cpp \
    gui/wizards/PrimerCreatorWizard.cpp \
    primer/PrimerSearchParameters.cpp \
    gui/Services/SpinBoxRangeLinker.cpp \
    gui/Services/DoubleSpinBoxRangeLinker.cpp \
    primer/Primer.cpp \
    primer/ThermodynamicCalculator.cpp \
    primer/PrimerPair.cpp \
    primer/PrimerPairFinder.cpp \
    primer/SignalPrimerPairFinder.cpp \
    primer/ThreePrimeInput.cpp \
    primer/PrimerPairModel.cpp \
    primer/RestrictionEnzymeTableModel.cpp \
    primer/RebaseParser.cpp \
    primer/RestrictionEnzymeLineEdit.cpp \
    primer/DnaSequenceValidator.cpp \
    primer/RestrictionEnzymeBrowserDialog.cpp \
    core/export/msa/FastaMsaExporter.cpp \
    core/export/msa/ClustalMsaExporter.cpp \
    gui/painting/SvgGeneratorEngine.cpp \
    primer/ThermodynamicConstants.cpp \
    primer/PrimerFactory.cpp \
    primer/PrimerPairFactory.cpp \
    core/DnaPattern.cpp \
    primer/DimerScoreCalculator.cpp \
    core/util/ClosedIntRange.cpp \
    primer/AbstractPrimerPairModel.cpp \
    gui/models/FilterColumnProxyModel.cpp \
    gui/widgets/KalignMsaBuilderOptionsWidget.cpp \
    core/constants/MsaBuilderIds.cpp \
    gui/widgets/EmptyMsaBuilderOptionsWidget.cpp \
    gui/factories/MsaBuilderOptionsWidgetFactory.cpp \
    core/factories/MsaBuilderFactory.cpp \
    gui/forms/dialogs/MsaAlignerOptionsDialog.cpp \
    gui/widgets/ClustalWMsaBuilderOptionsWidget.cpp \
    gui/widgets/FastPairwiseParametersDialog.cpp \
    gui/widgets/SlowPairwiseParametersDialog.cpp \
    primer/DnaSeqPrimerVectorMutator.cpp \
    primer/DnaSeqCommandPrimerMutator.cpp \
    primer/AppendPrimersToDnaSeqCommand.cpp \
    primer/PrimerMutator.cpp \
    primer/SetDnaSeqPrimerNameCommand.cpp \
    primer/DnaSeqPrimerModel.cpp \
    primer/AbstractPrimerModel.cpp \
    core/DataSources/Crud/DbPrimerSearchParametersCache.cpp \
    gui/forms/flextabs/PrimersViewTab.cpp \
    gui/forms/dialogs/NewPrimerDialog.cpp \
    primer/RemoveDnaSeqPrimersCommand.cpp \
    gui/delegates/RestrictionEnzymeAccentDelegate.cpp \
    gui/painting/features/AbstractPrimerItem.cpp \
    gui/painting/features/ForwardPrimerItem.cpp \
    gui/painting/features/ReversePrimerItem.cpp \
    gui/factories/PrimerItemFactory.cpp \
    gui/Services/DnaSeqFeatureItemSynchronizer.cpp \
    gui/forms/dialogs/NewSequenceEntityDialog.cpp \
    gui/Services/InvalidCharsHighlighter.cpp \
    core/factories/DnaSeqFactory.cpp \
    core/factories/DynamicSeqFactory.cpp \
    core/factories/AminoSeqFactory.cpp \
    gui/forms/dialogs/PrimerSearchParametersInfoDialog.cpp \
    gui/forms/dialogs/AboutDialog.cpp \
    core/data/CommonBioSymbolGroups.cpp \
    gui/widgets/ConsensusView.cpp \
    gui/forms/dialogs/ConsensusOptionsDialog.cpp \
    gui/models/MsaConsensusModel.cpp \
    gui/widgets/SingleColumnTableView.cpp \
    gui/widgets/ConsensusLabelTableView.cpp \
    gui/delegates/ConsensusLabelDelegate.cpp \
    gui/widgets/FontAndSizeChooser.cpp \
    core/Services/LicenseValidator.cpp \
    core/factories/LicenseFactory.cpp \
    gui/forms/dialogs/LicenseInfoDialog.cpp \
    gui/wizards/LicenseWizard.cpp \
    gui/Commands/Msa/MoveRowsCommand.cpp \
    gui/forms/dialogs/ConsensusGroupsDialog.cpp \
    gui/models/ConsensusGroupsModel.cpp

HEADERS  += \
    core/DataMappers/AbstractAnonSeqMapper.h \
    core/DataMappers/AbstractEntityMapper.h \
    core/DataMappers/AminoMsaMapper.h \
    core/DataMappers/AminoSeqMapper.h \
    core/DataMappers/AnonSeqMapper.h \
    core/DataMappers/DnaSeqMapper.h \
    core/DataMappers/GenericEntityMapper.h \
    core/DataMappers/IAnonSeqMapper.h \
    core/DataMappers/IEntityMapper.h \
    core/DataMappers/IMsaMapper.h \
    core/DataMappers/MsaMapper.h \
    core/DataSources/Crud/AbstractDbEntityCrud.h \
    core/DataSources/Crud/DbAminoMsaCrud.h \
    core/DataSources/Crud/DbAminoSeqCrud.h \
    core/DataSources/Crud/DbAstringCrud.h \
    core/DataSources/Crud/DbDstringCrud.h \
    core/DataSources/Crud/DbDnaSeqCrud.h \
    core/DataSources/Crud/IAnonSeqEntityCrud.h \
    core/DataSources/Crud/IDbEntityCrud.h \
    core/DataSources/Crud/IEntityCrud.h \
    core/DataSources/Crud/IMsaCrud.h \
    core/DataSources/AbstractAdocSource.h \
    core/DataSources/AbstractDbSource.h \
    core/DataSources/IAdocSource.h \
    core/DataSources/IDbSource.h \
    core/DataSources/SqliteAdocSource.h \
    core/Detectors/AlphabetDetector.h \
    core/Detectors/ConsensusAlphabetDetector.h \
    core/Detectors/DataFormatDetector.h \
    core/Detectors/IAlphabetDetector.h \
    core/Detectors/IDataFormatDetector.h \
    core/Entities/AbstractAnonSeq.h \
    core/Entities/AbstractBasicEntity.h \
    core/Entities/AbstractEntity.h \
    core/Entities/AbstractMsa.h \
    core/Entities/AbstractSeq.h \
    core/Entities/AminoMsa.h \
    core/Entities/AminoSeq.h \
    core/Entities/Astring.h \
    core/Entities/DnaSeq.h \
    core/Entities/Dstring.h \
    core/Entities/IBasicEntity.h \
    core/Entities/IEntity.h \
    core/Parsers/AbstractSequenceParser.h \
    core/Parsers/ClustalParser.h \
    core/Parsers/FastaParser.h \
    core/Parsers/ISequenceParser.h \
    core/PODs/SequenceParseResultPod.h \
    core/PODs/SimpleSeqPod.h \
    core/PODs/SubseqChangePod.h \
    core/Repositories/AnonSeqRepository.h \
    core/Repositories/GenericRepository.h \
    core/Repositories/IAnonSeqRepository.h \
    core/Repositories/IMsaRepository.h \
    core/Repositories/IRepository.h \
    core/Repositories/MsaRepository.h \
    core/Services/PodEntityService.h \
    core/Services/SymbolStringCalculator.h \
    core/ValueObjects/Coil.h \
    core/ValueObjects/IFeature.h \
    core/ValueObjects/Seg.h \
    core/util/AbstractNumberGenerator.h \
    core/util/ClosedIntRange.h \
    core/util/DecrementNumberGenerator.h \
    core/util/INumberGenerator.h \
    core/util/IncrementNumberGenerator.h \
    core/util/IntNumberGenerator.h \
    core/util/MsaAlgorithms.h \
    core/util/NumberGenerator.h \
    core/util/QVariantLessGreaterThan.h \
    core/AbstractLiveCharCountDistribution.h \
    core/Adoc.h \
    core/BioString.h \
    core/BioStringValidator.h \
    core/BioSymbol.h \
    core/BioSymbolGroup.h \
    core/CharCountDistribution.h \
    core/DataFormat.h \
    core/LiveMsaCharCountDistribution.h \
    core/LiveSymbolString.h \
    core/MpttNode.h \
    core/Msa.h \
    core/ObservableMsa.h \
    core/Seq.h \
    core/Subseq.h \
    core/TreeNode.h \
    core/UngappedSubseq.h \
    core/constants.h \
    core/enums.h \
    core/global.h \
    core/macros.h \
    core/metatypes.h \
    core/misc.h \
    core/types.h \
    graphics/AbstractCharPixelMetrics.h \
    graphics/AbstractTextRenderer.h \
    graphics/BasicTextRenderer.h \
    graphics/BioSymbolColorScheme.h \
    graphics/CharColorScheme.h \
    graphics/CharPixelMetrics.h \
    graphics/CharPixelMetricsF.h \
    graphics/ExactTextRenderer.h \
    graphics/PositionalMsaColorProvider.h \
    graphics/SymbolColorScheme.h \
    graphics/SymbolColorProvider.h \
    graphics/TextColorStyle.h \
    graphics/TextImageRenderer.h \
    graphics/TextPixmapRenderer.h \
    graphics/graphics_misc.h \
    gui/Commands/InsertAdocTreeNodesCommand.h \
    gui/Commands/MoveAdocTreeNodesCommand.h \
    gui/Commands/RemoveAdocTreeNodesCommand.h \
    gui/Commands/SetEntityDataCommand.h \
    gui/Services/AdocTreeNodeEraserService.h \
    gui/delegates/LineEditDelegate.h \
    gui/delegates/ClipboardStateItemDelegate.h \
    gui/forms/MainWindow.h \
    gui/forms/MsaWindow.h \
    gui/forms/SelectGroupNodeDialog.h \
    gui/models/ColumnAdapters/AbstractColumnAdapter.h \
    gui/models/ColumnAdapters/AminoMsaColumnAdapter.h \
    gui/models/ColumnAdapters/AminoSeqColumnAdapter.h \
    gui/models/ColumnAdapters/DnaSeqColumnAdapter.h \
    gui/models/ColumnAdapters/IColumnAdapter.h \
    gui/models/AbstractMultiEntityTableModel.h \
    gui/models/AdocTreeModel.h \
    gui/models/AdocTreeNodeFilterModel.h \
    gui/models/MultiSeqTableModel.h \
    gui/util/ModelIndexRange.h \
    gui/widgets/AbstractMsaView.h \
    gui/widgets/AdocTreeView.h \
    gui/widgets/MultiSeqTableView.h \
    gui/widgets/NativeMsaView.h \
    gui/SequenceImporter.h \
    gui/gui_misc.h \
    gui/Commands/Msa/CollapseMsaRectLeftCommand.h \
    gui/Commands/Msa/CollapseMsaRectRightCommand.h \
    gui/Commands/Msa/TrimRowsLeftCommand.h \
    gui/Commands/Msa/TrimRowsRightCommand.h \
    gui/Commands/Msa/ExtendRowsLeftCommand.h \
    gui/Commands/Msa/ExtendRowsRightCommand.h \
    gui/Commands/Msa/LevelRowsLeftCommand.h \
    gui/Commands/Msa/LevelRowsRightCommand.h \
    gui/Commands/AbstractSkipFirstRedoCommand.h \
    gui/Commands/Msa/SlideMsaRectCommand.h \
    gui/Commands/Msa/InsertGapColumnsCommand.h \
    gui/Commands/Msa/RemoveAllGapColumnsCommand.h \
    gui/Commands/Msa/SetSubseqStartCommand.h \
    gui/models/MsaSubseqTableModel.h \
    gui/Commands/Msa/SortMsaCommand.h \
    graphics/ColorSchemes.h \
    gui/models/ColumnAdapters/IFilterColumnAdapter.h \
    gui/models/ColumnAdapters/FilterColumnAdapter.h \
    gui/forms/AminoMsaWindow.h \
    gui/Commands/Msa/SetSubseqStopCommand.h \
    gui/Commands/Msa/RemoveRowsCommand.h \
    core/machines/events/CustomEventTypes.h \
    core/Parsers/SignalSequenceParser.h \
    gui/MsaTools/IMsaTool.h \
    gui/MsaTools/AbstractMsaTool.h \
    gui/MsaTools/HandMsaTool.h \
    gui/EventFilters/WidgetFocusObserver.h \
    gui/MsaTools/SelectMsaTool.h \
    gui/util/PointRectMapper.h \
    gui/MsaTools/ZoomMsaTool.h \
    gui/MsaTools/GapMsaTool.h \
    gui/painting/IRenderEngine.h \
    gui/painting/NativeRenderEngine.h \
    gui/painting/AbstractRenderEngine.h \
    gui/painting/GLRenderEngine.h \
    graphics/CharColorProvider.h \
    gui/widgets/AgDoubleSpinBox.h \
    gui/widgets/PercentSpinBox.h \
    gui/Commands/SetGroupLabelCommand.h \
    core/DataSources/Crud/DbDnaMsaCrud.h \
    core/Entities/DnaMsa.h \
    core/DataMappers/DnaMsaMapper.h \
    gui/models/ColumnAdapters/DnaMsaColumnAdapter.h \
    gui/forms/DnaMsaWindow.h \
    core/InfoContentDistribution.h \
    core/LiveInfoContentDistribution.h \
    core/_Mocks/MockLiveCharCountDistribution.h \
    core/_Mocks/MockCharCountDistributions.h \
    gui/painting/gitems/InfoUnitItem.h \
    core/PODs/InfoUnit.h \
    gui/painting/gitems/LogoBarsItem.h \
    core/Services/PsiBlastWrapper.h \
    core/Services/PsiBlastStructureTool.h \
    core/PODs/Q3Prediction.h \
    core/Services/FannWrapper.h \
    core/Services/NNStructureTool.h \
    core/PODs/Pssm.h \
    core/PODs/NormalizedPssm.h \
    core/Parsers/PssmParser.h \
    core/util/PssmUtil.h \
    gui/Services/Tasks/ITask.h \
    gui/Services/TaskManager.h \
    gui/Services/Tasks/Task.h \
    gui/models/TaskModel.h \
    core/PointerTreeNode.h \
    gui/Services/Tasks/TaskTreeNode.h \
    gui/Services/Tasks/PredictSecondaryTask.h \
    graphics/LinearColorScheme.h \
    graphics/SecondaryStructureColorProvider.h \
    graphics/PODs/HSVA.h \
    graphics/LinearColorStyle.h \
    core/Entities/EntityFlags.h \
    gui/widgets/EventSignalWidget.h \
    gui/widgets/EventSignalGraphicsView.h \
    gui/widgets/MsaDataColumnWidget.h \
    gui/widgets/AbstractMsaSideWidget.h \
    gui/widgets/MsaStartStopSideWidget.h \
    gui/widgets/MsaRulerWidget.h \
    gui/widgets/SinglePixmapMsaView.h \
    gui/painting/gitems/LogoItem.h \
    gui/painting/gitems/AntiToggleLineItem.h \
    gui/widgets/MsaVertSelectionBar.h \
    gui/widgets/MsaTableView.h \
    gui/delegates/MsaLineEditDelegate.h \
    gui/MsaTools/MsaToolTypes.h \
    gui/Commands/Msa/AbstractCollapseMsaRectCommand.h \
    gui/Commands/CommandIds.h \
    core/Services/AbstractProcessWrapper.h \
    core/PODs/BlastDatabaseMetaPod.h \
    core/Services/BlastDatabaseFinder.h \
    gui/models/BlastDatabaseModel.h \
    gui/models/AbstractBaseTreeModel.h \
    gui/forms/BlastDatabaseWindow.h \
    core/Services/IBlastDatabaseInstaller.h \
    core/Services/AbstractBlastDatabaseInstaller.h \
    core/Services/NcbiBlastDatabaseInstaller.h \
    core/util/Pipe.h \
    core/util/GunzipPipe.h \
    core/util/GzipPipe.h \
    core/util/UntarSink.h \
    core/PODs/TarHeaderPod.h \
    core/util/TarUtil.h \
    core/machines/events/BoolEvent.h \
    core/machines/transitions/BoolTransition.h \
    gui/forms/BlastDatabaseDownloadWindow.h \
    gui/Commands/ConditionalUndoCommand.h \
    gui/Services/TaskAdocConnector.h \
    gui/Services/Tasks/IEntityBuilderTask.h \
    gui/Commands/InsertBlastReportsCommand.h \
    core/Entities/TransientTask.h \
    gui/models/ColumnAdapters/TransientTaskColumnAdapter.h \
    core/Entities/BlastReport.h \
    core/PODs/HspPod.h \
    core/PODs/HitPod.h \
    gui/Services/Tasks/BlastTask.h \
    gui/forms/BlastDialog.h \
    gui/models/CustomRoles.h \
    core/Repositories/MemoryOnlyRepository.h \
    core/DataSources/Crud/DbBlastReportCrud.h \
    gui/models/ColumnAdapters/BlastReportColumnAdapter.h \
    core/DataMappers/BlastReportMapper.h \
    core/DataSources/Crud/IBlastReportCrud.h \
    core/ValueTreeNode.h \
    core/AdocTreeNode.h \
    core/Mptt.h \
    core/BaseValueTreeNode.h \
    core/BasePointerTreeNode.h \
    core/Parsers/xml/BlastXmlHandler.h \
    gui/models/BlastReportModel.h \
    gui/delegates/MultiLineDelegate.h \
    core/PODs/BlastDatabaseSpec.h \
    core/Services/BlastSequenceFetcher.h \
    core/Services/BlastDbCmdBase.h \
    gui/Services/BlastSequenceImporter.h \
    core/PODs/OptionSpec.h \
    core/util/OptionProfile.h \
    core/PODs/Option.h \
    core/util/OptionSet.h \
    core/constants/PsiBlastConstants.h \
    core/Services/AbstractMsaBuilder.h \
    core/PODs/IdBioString.h \
    core/constants/KalignConstants.h \
    core/Services/KalignMsaBuilder.h \
    gui/Services/Tasks/BuildMsaTask.h \
    gui/Commands/InsertTaskNodesCommand.h \
    core/constants/ClustalWConstants.h \
    core/Services/ClustalWMsaBuilder.h \
    gui/BasicApplication.h \
    gui/forms/PredictSecondaryDialog.h \
    gui/wizards/MakeBlastDatabaseWizard.h \
    core/Services/MakeBlastDatabaseWrapper.h \
    core/constants/MakeBlastDbConstants.h \
    gui/widgets/FlexTabWidget.h \
    gui/forms/flextabs/BlastViewTab.h \
    gui/widgets/views/WheelZoomGraphicsView.h \
    gui/painting/gitems/AbstractLinearItem.h \
    gui/painting/gitems/AbstractRangeItem.h \
    gui/painting/gitems/AminoSeqItem.h \
    gui/painting/gitems/RangeHandleItem.h \
    gui/painting/gitems/BioStringItem.h \
    gui/painting/gitems/SeqBioStringItem.h \
    gui/painting/gitems/AbstractSeqItem.h \
    gui/painting/LinearGraphicsScene.h \
    gui/painting/gitems/RangeHandlePairItem.h \
    gui/painting/gitems/LinearRulerItem.h \
    gui/painting/gitems/DnaSeqItem.h \
    gui/delegates/SpinBoxDelegate.h \
    gui/widgets/SequenceTextView.h \
    gui/Services/HeaderColumnSelector.h \
    gui/wizards/PrimerCreatorWizard.h \
    primer/PrimerSearchParameters.h \
    core/util/Range.h \
    gui/Services/SpinBoxRangeLinker.h \
    gui/Services/DoubleSpinBoxRangeLinker.h \
    primer/Primer.h \
    primer/ThermodynamicCalculator.h \
    primer/PrimerPair.h \
    primer/PrimerPairFinder.h \
    primer/SignalPrimerPairFinder.h \
    primer/ThreePrimeInput.h \
    primer/PrimerPairModel.h \
    primer/RestrictionEnzymeTableModel.h \
    primer/RebaseParser.h \
    primer/RestrictionEnzyme.h \
    primer/RestrictionEnzymeLineEdit.h \
    primer/DnaSequenceValidator.h \
    primer/RestrictionEnzymeBrowserDialog.h \
    core/export/msa/FastaMsaExporter.h \
    core/export/msa/ClustalMsaExporter.h \
    core/export/msa/IMsaExporter.h \
    gui/painting/SvgGeneratorEngine.h \
    primer/ThermodynamicConstants.h \
    primer/PrimerFactory.h \
    primer/PrimerPairFactory.h \
    core/DnaPattern.h \
    primer/DimerScoreCalculator.h \
    core/ValueObjects/ValueObject.h \
    primer/AbstractPrimerPairModel.h \
    gui/models/FilterColumnProxyModel.h \
    gui/widgets/IMsaBuilderOptionsWidget.h \
    gui/widgets/KalignMsaBuilderOptionsWidget.h \
    core/constants/MsaBuilderIds.h \
    gui/widgets/EmptyMsaBuilderOptionsWidget.h \
    gui/factories/MsaBuilderOptionsWidgetFactory.h \
    core/factories/MsaBuilderFactory.h \
    gui/forms/dialogs/MsaAlignerOptionsDialog.h \
    gui/widgets/ClustalWMsaBuilderOptionsWidget.h \
    gui/widgets/FastPairwiseParametersDialog.h \
    gui/widgets/SlowPairwiseParametersDialog.h \
    primer/DnaSeqPrimerVectorMutator.h \
    primer/DnaSeqCommandPrimerMutator.h \
    primer/AppendPrimersToDnaSeqCommand.h \
    primer/IPrimerMutator.h \
    primer/PrimerMutator.h \
    primer/SetDnaSeqPrimerNameCommand.h \
    primer/DnaSeqPrimerModel.h \
    primer/AbstractPrimerModel.h \
    core/DataSources/Crud/DbPrimerSearchParametersCache.h \
    gui/forms/flextabs/PrimersViewTab.h \
    gui/forms/dialogs/NewPrimerDialog.h \
    primer/RemoveDnaSeqPrimersCommand.h \
    gui/delegates/RestrictionEnzymeAccentDelegate.h \
    gui/painting/features/AbstractFeaureItem.h \
    gui/painting/features/AbstractPrimerItem.h \
    gui/painting/features/FeatureTypes.h \
    gui/painting/features/ForwardPrimerItem.h \
    gui/painting/features/AbstractRangeFeatureItem.h \
    gui/painting/features/ReversePrimerItem.h \
    gui/Services/DnaSeqFeatureItemSynchronizer.h \
    gui/factories/PrimerItemFactory.h \
    gui/forms/dialogs/NewSequenceEntityDialog.h \
    gui/Services/InvalidCharsHighlighter.h \
    core/factories/DnaSeqFactory.h \
    core/factories/AbstractSeqFactory.h \
    core/factories/DynamicSeqFactory.h \
    core/factories/AminoSeqFactory.h \
    gui/forms/dialogs/PrimerSearchParametersInfoDialog.h \
    gui/forms/dialogs/AboutDialog.h \
    core/data/CommonBioSymbolGroups.h \
    gui/widgets/ConsensusView.h \
    gui/widgets/VerticalMsaMarginWidget.h \
    gui/forms/dialogs/ConsensusOptionsDialog.h \
    gui/models/MsaConsensusModel.h \
    gui/widgets/SingleColumnTableView.h \
    gui/widgets/ConsensusLabelTableView.h \
    gui/delegates/ConsensusLabelDelegate.h \
    gui/widgets/FontAndSizeChooser.h \
    core/Services/LicenseValidator.h \
    core/factories/LicenseFactory.h \
    gui/forms/dialogs/LicenseInfoDialog.h \
    gui/wizards/LicenseWizard.h \
    gui/Commands/Msa/MoveRowsCommand.h \
    gui/forms/dialogs/ConsensusGroupsDialog.h \
    gui/models/ConsensusGroupsModel.h \
    gui/delegates/RegexDelegate.h

FORMS    += gui/forms/MainWindow.ui \
    gui/forms/SelectGroupNodeDialog.ui \
    gui/forms/MsaWindow.ui \
    gui/forms/BlastDatabaseWindow.ui \
    gui/forms/BlastDatabaseDownloadWindow.ui \
    gui/forms/BlastDialog.ui \
    gui/forms/PredictSecondaryDialog.ui \
    gui/forms/flextabs/BlastViewTab.ui \
    primer/RestrictionEnzymeBrowserDialog.ui \
    gui/widgets/KalignMsaBuilderOptionsWidget.ui \
    gui/widgets/EmptyMsaBuilderOptionsWidget.ui \
    gui/forms/dialogs/MsaAlignerOptionsDialog.ui \
    gui/widgets/ClustalWMsaBuilderOptionsWidget.ui \
    gui/widgets/FastPairwiseParametersDialog.ui \
    gui/widgets/SlowPairwiseParametersDialog.ui \
    gui/forms/flextabs/PrimerViewTab.ui \
    gui/forms/dialogs/NewPrimerDialog.ui \
    gui/forms/dialogs/NewSequenceEntityDialog.ui \
    gui/forms/dialogs/PrimerSearchParametersInfoDialog.ui \
    gui/forms/dialogs/AboutDialog.ui \
    gui/forms/dialogs/ConsensusOptionsDialog.ui \
    gui/forms/dialogs/LicenseInfoDialog.ui \
    gui/forms/dialogs/ConsensusGroupsDialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    GeneralTests.txt \
    BetaLicense.html









































