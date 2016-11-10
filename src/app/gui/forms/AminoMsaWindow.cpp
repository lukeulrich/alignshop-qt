/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QSettings>

#include <QtGui/QAbstractButton>
#include <QtGui/QActionGroup>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>
#include <QtGui/QScrollBar>

#include "AminoMsaWindow.h"
#include "BlastDatabaseWindow.h"
#include "PredictSecondaryDialog.h"

#include "../models/BlastDatabaseModel.h"
#include "../models/ColumnAdapters/AminoSeqColumnAdapter.h"
#include "../models/ColumnAdapters/FilterColumnAdapter.h"
#include "../models/MsaSubseqTableModel.h"

#include "../../core/data/CommonBioSymbolGroups.h"
#include "../../core/Entities/AbstractMsa.h"
#include "../../core/LiveInfoContentDistribution.h"
#include "../../core/LiveMsaCharCountDistribution.h"
#include "../../core/LiveSymbolString.h"
#include "../../core/Services/SymbolStringCalculator.h"
#include "../../graphics/ColorSchemes.h"
#include "../../graphics/CharColorProvider.h"
#include "../../graphics/SecondaryStructureColorProvider.h"
#include "../../graphics/SymbolColorProvider.h"
#include "../../graphics/AbstractTextRenderer.h"
#include "../painting/IRenderEngine.h"
#include "../painting/gitems/LogoBarsItem.h"
#include "../painting/gitems/LogoItem.h"
#include "../gui_misc.h"

#include "ui_MsaWindow.h"


#include "../models/MsaConsensusModel.h"
#include "../widgets/ConsensusView.h"
#include "../widgets/ConsensusLabelTableView.h"
#include "../forms/dialogs/ConsensusOptionsDialog.h"
#include "../forms/dialogs/ConsensusGroupsDialog.h"
#include "../delegates/ConsensusLabelDelegate.h"

#include <QtCore/QScopedPointer>
#include "../../core/constants.h"
#include "../../core/Adoc.h"
#include "../../core/ObservableMsa.h"
#include "../../core/Subseq.h"
#include "../Services/TaskManager.h"
#include "../Services/Tasks/PredictSecondaryTask.h"
#include "../Services/Tasks/Task.h"
#include "../Services/Tasks/TaskTreeNode.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adoc [Adoc *]
  * @param taskManager [TaskManager *]
  * @param blastDatabaseModel [BlastDatabaseModel *]
  * @param parent [QWidget *]
  */
AminoMsaWindow::AminoMsaWindow(Adoc *adoc,
                               TaskManager *taskManager,
                               BlastDatabaseModel *blastDatabaseModel,
                               BlastDatabaseWindow *blastDatabaseWindow,
                               QWidget *parent)
    : MsaWindow(adoc, taskManager, parent),
      blastDatabaseModel_(blastDatabaseModel),
      blastDatabaseWindow_(blastDatabaseWindow),
      predictSecondaryDialog_(nullptr),
      aminoSeqColumnAdapter_(nullptr),
      filterColumnAdapter_(nullptr),
      liveClustalSymbolString_(nullptr),
      liveInfoContentDistribution_(nullptr),
      msaConsensusModel_(nullptr),
      consensusView_(nullptr),
      consensusLabelTableView_(nullptr),
      colorProvidersGroup_(nullptr),
      defaultColorProviderAction_(nullptr),
      predictSecondaryStructureAction_(nullptr),
      useCustomConsensusGroups_(false)
{
    ASSERT(blastDatabaseModel_ != nullptr);
    ASSERT(blastDatabaseWindow_ != nullptr);
    ASSERT(taskManager != nullptr);

    setWindowIcon(QIcon(":aliases/images/icons/amino-msa"));

    aminoSeqColumnAdapter_ = new AminoSeqColumnAdapter(this);
    aminoSeqColumnAdapter_->setUndoStack(undoStack_);

    filterColumnAdapter_ = new FilterColumnAdapter(this);
    filterColumnAdapter_->setSourceAdapter(aminoSeqColumnAdapter_);
    filterColumnAdapter_->exclude(AminoSeqColumnAdapter::eIdColumn);
    filterColumnAdapter_->exclude(AminoSeqColumnAdapter::eStartColumn);
    filterColumnAdapter_->exclude(AminoSeqColumnAdapter::eStopColumn);

    setupColorProviders();

    logoItem()->logoBarsItem()->setColorScheme(ColorSchemes::kLogoAminoScheme);

    // -------------------------------------------------
    // Setup the predict secondary structure menu option
    ui_->menu_Visualizations->addSeparator();
    predictSecondaryStructureAction_ = ui_->menu_Visualizations->addAction("Predict secondary structure");
    connect(predictSecondaryStructureAction_, SIGNAL(triggered()), SLOT(onPredictSecondaryStructureActionTriggered()));

    // Until a valid msa has been set, this action should remain disabled
    predictSecondaryStructureAction_->setEnabled(false);

    // To properly update the msa when a task completes, we must watch for predict secondary tasks that complete
    connect(taskManager, SIGNAL(taskAboutToStart(ITask*)), SLOT(onTaskAboutToStart(ITask*)));

    // -------------------------------------------------
    // Consensus control and actions
    msaConsensusModel_ = new MsaConsensusModel(this);
    msaConsensusModel_->setThresholds(QVector<double>() << .9 << .8 << .7 << .6 << .5);

    consensusLabelTableView_ = new ConsensusLabelTableView;
    QVBoxLayout *containerLayout = new QVBoxLayout;
    containerLayout->setContentsMargins(0, 0, 0, 0);
    ui_->bottomMarginWidgetContainer_->setLayout(containerLayout);
    ui_->bottomMarginWidgetContainer_->layout()->addWidget(consensusLabelTableView_);
    consensusLabelTableView_->setColumn(MsaConsensusModel::eFriendlyThresholdColumn);
    consensusLabelTableView_->setModel(msaConsensusModel_);
    consensusLabelTableView_->setIgnoreShortcutOverrides(actionShortCuts());
    consensusLabelTableView_->setIgnoreShortcutKey('+');
    consensusLabelTableView_->setIgnoreShortcutKey('=');
    connect(ui_->msaView, SIGNAL(fontChanged()), SLOT(updateConsensusLabelFont()));
    connect(ui_->msaView, SIGNAL(zoomChanged(double)), SLOT(updateConsensusLabelFont()));
    consensusLabelTableView_->setItemDelegateForColumn(consensusLabelTableView_->column(), new ConsensusLabelDelegate(this));

    consensusView_ = new ConsensusView(ui_->msaView);
    consensusView_->hide();     // Initially not shown
    consensusView_->setConsensusModel(msaConsensusModel_);
    consensusView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    consensusView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    consensusView_->setRenderEngine(ui_->msaView->renderEngine());
    connect(ui_->msaView, SIGNAL(renderXShiftChanged(qreal)), consensusView_, SLOT(setRenderXShift(qreal)));
    connect(ui_->msaView->horizontalScrollBar(), SIGNAL(valueChanged(int)), consensusView_->horizontalScrollBar(), SLOT(setValue(int)));
    ui_->msaView->setVerticalMsaMarginWidget(consensusView_);

    QMenu *consensusMenu = new QMenu("Consensus");
    QAction *consensusAction = consensusMenu->addAction("Visible");
    consensusAction->setCheckable(true);
    connect(consensusAction, SIGNAL(triggered(bool)), SLOT(onConsensusActionToggled(bool)));
    QAction *consensusOptionsAction = consensusMenu->addAction("Edit thresholds...");
    consensusOptionsAction->setIcon(QIcon(":aliases/images/icons/okteta"));
    connect(consensusOptionsAction, SIGNAL(triggered()), SLOT(onConsensusThresholdsActionTriggered()));
    QAction *editConsensusGroupsAction = consensusMenu->addAction("Edit groups...");
    connect(editConsensusGroupsAction, SIGNAL(triggered()), SLOT(onEditConsensusGroupsActionTriggered()));
    ui_->menu_View->insertMenu(sequenceLabelsAction_, consensusMenu);

    QSettings settings;
    settings.beginGroup("AminoMsaWindow");
    useCustomConsensusGroups_ = settings.value("UseCustomConsensusGroups", false).toBool();
    if (settings.contains("CustomConsensusGroups"))
        customConsensusGroup_ = qvariant_cast<BioSymbolGroup>(settings.value("CustomConsensusGroups"));
    if (customConsensusGroup_.isEmpty())
        customConsensusGroup_ = constants::CommonBioSymbolGroups::defaultConsensusSymbolGroup();
    settings.endGroup();
}

AminoMsaWindow::~AminoMsaWindow()
{
    QSettings settings;
    settings.beginGroup("AminoMsaWindow");
    settings.setValue("UseCustomConsensusGroups", useCustomConsensusGroups_);
    settings.setValue("CustomConsensusGroups", QVariant::fromValue(customConsensusGroup_));
    settings.endGroup();
}

/**
  * Subclasses should return a valid pointer here if they want too.
  */
IColumnAdapter *AminoMsaWindow::subseqEntityColumnAdapter() const
{
    return filterColumnAdapter_;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
LiveInfoContentDistribution *AminoMsaWindow::liveInfoContentDistribution() const
{
    return liveInfoContentDistribution_;
}


/**
  * @param abstractMsa [AbstractMsa *]
  */
void AminoMsaWindow::setMsaEntityImpl(const AbstractMsaSPtr &abstractMsa)
{
    ASSERT(abstractMsa->type() == eAminoMsaEntity);

    // --------------------
    // Setup the logo scene
    delete liveInfoContentDistribution_;
    liveInfoContentDistribution_ = nullptr;
    if (liveMsaCharCountDistribution() != nullptr)
    {
        liveInfoContentDistribution_ = new LiveInfoContentDistribution(liveMsaCharCountDistribution(), 20, true, this);
        logoItem()->logoBarsItem()->setLiveInfoContentDistribution(liveInfoContentDistribution_);

        connect(liveInfoContentDistribution_, SIGNAL(columnsInserted(ClosedIntRange)), SLOT(updateLogoSceneRect()));
        connect(liveInfoContentDistribution_, SIGNAL(columnsRemoved(ClosedIntRange)), SLOT(updateLogoSceneRect()));

        // Trigger the scene rect to update in order to set the horizontal scroll bar to its left most position
        updateLogoSceneRect();
        ui_->logoGraphicsView->horizontalScrollBar()->setValue(ui_->logoGraphicsView->horizontalScrollBar()->minimum());
    }

    // -------------------------------------------------------
    // Horizontal header tweaks for the msa subseq table model
    //
    // Note: it is vital that these UI changes are done after MsaWindow::setMsaEntity because that method sets the
    // msa and subseqColumnAdapter of the MsaSubseqTableModel which determines the actual headers.
    //
    // Hide the type, notes, and sequence columns
    ui_->subseqTableView->horizontalHeader()->hideSection(mapFromColumnAdapter(AminoSeqColumnAdapter::eTypeColumn));
    ui_->subseqTableView->horizontalHeader()->hideSection(mapFromColumnAdapter(AminoSeqColumnAdapter::eNotesColumn));
    ui_->subseqTableView->horizontalHeader()->hideSection(mapFromColumnAdapter(AminoSeqColumnAdapter::eSequenceColumn));

    // Move the name column to be the first column in the list
    ui_->subseqTableView->horizontalHeader()->moveSection(mapFromColumnAdapter(AminoSeqColumnAdapter::eNameColumn), 0);
    ui_->subseqTableView->resizeColumnToContents(mapFromColumnAdapter(AminoSeqColumnAdapter::eSourceColumn));

    ui_->menu_Visualizations->setEnabled(abstractMsa != nullptr);

    // Trigger the coloring for the currently selected color scheme
    QAction *currentColorAction = colorProvidersGroup_->checkedAction();
    if (currentColorAction)
        currentColorAction->activate(QAction::Trigger);
    else
        // Or use the default color provider if none is selected
        defaultColorProviderAction_->setChecked(true);


    // -------------------------------------------------------
    // Now that we have a valid msa entity, enable the secondary structure prediction action
    predictSecondaryStructureAction_->setEnabled(true);


    // -------------------------------------------------------
    // Choose the appropriate column we want to show in the label view
    ui_->labelView->setColumn(mapFromColumnAdapter(AminoSeqColumnAdapter::eNameColumn));

    updateConsensusModel();
    updateConsensusLabelFont();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void AminoMsaWindow::onBasicColorActionTriggered()
{
    // By setting the color provider to null, it will use the default renderer which is simply black on white.
    ui_->msaView->setPositionalMsaColorProvider(nullptr);
}

/**
  * @param checked [bool]
  */
void AminoMsaWindow::onClustalColorActionToggled(bool checked)
{
    // Only should be null in the event that the msa changes or upon initialization
    if (msaEntity() == nullptr || msaEntity()->msa() == nullptr)
        return;

    if (checked)
    {
        using namespace constants::CommonBioSymbolGroups;
        liveClustalSymbolString_ = new LiveSymbolString(liveMsaCharCountDistribution(),
                                                 SymbolStringCalculator(kClustalAminoSymbolGroup, ' '),
                                                 this);
        ui_->msaView->setPositionalMsaColorProvider(new SymbolColorProvider(liveClustalSymbolString_, ColorSchemes::kClustalAminoScheme));
    }
    else
    {
        // This means that the clustal color action is no longer checked. Free the live char count distribution and
        // symbol string
        delete liveClustalSymbolString_;
        liveClustalSymbolString_ = nullptr;
    }
}

/**
  */
void AminoMsaWindow::onZappaColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new CharColorProvider(ColorSchemes::kZappoAminoScheme));
}

/**
  */
void AminoMsaWindow::onTaylorColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new CharColorProvider(ColorSchemes::kTaylorAminoScheme));
}

/**
  */
void AminoMsaWindow::onHydroColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new CharColorProvider(ColorSchemes::kHydrophobocityScheme));
}

/**
  */
void AminoMsaWindow::onHelixColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new CharColorProvider(ColorSchemes::kHelixPropensityScheme));
}

/**
  */
void AminoMsaWindow::onStrandColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new CharColorProvider(ColorSchemes::kStrandPropensityScheme));
}

/**
  */
void AminoMsaWindow::onTurnColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new CharColorProvider(ColorSchemes::kTurnPropensityScheme));
}

void AminoMsaWindow::onBuriedColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new CharColorProvider(ColorSchemes::kBuriedIndexScheme));
}

/**
  */
void AminoMsaWindow::onSecondaryColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new SecondaryStructureColorProvider(ColorSchemes::kSecondaryStructureScheme));
}

/**
  */
void AminoMsaWindow::onPredictSecondaryStructureActionTriggered()
{
    // ---------------------------------------------------
    // Check that at least one amino blast database exists
    QVector<QPersistentModelIndex> blastDatabaseIndices = blastDatabaseModel_->aminoBlastDatabases();
    if (blastDatabaseIndices.isEmpty())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("BLAST database configuration needed");
        msgBox.setText("No protein BLAST databases have been configured. You must set one up in the BLAST "
                       "database manager before predicting secondary structure.");
        msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
        msgBox.button(QMessageBox::Open)->setText("Open BLAST Database Manager");
        if (msgBox.exec() == QMessageBox::Open)
        {
            blastDatabaseWindow_->show();
            blastDatabaseWindow_->raise();
            blastDatabaseWindow_->activateWindow();
        }

        return;
    }

    if (predictSecondaryDialog_ == nullptr)
        predictSecondaryDialog_ = new PredictSecondaryDialog(this);

    predictSecondaryDialog_->setMaxThreads(taskManager()->maxThreads());
    predictSecondaryDialog_->setBlastDatabaseIndices(blastDatabaseIndices);
    if (!predictSecondaryDialog_->exec())
        return;

    // Determine the list of amino seqs without secondary structure data and submit those
    Adoc *adoc = MsaWindow::adoc();
    QScopedPointer<TaskTreeNode> group(new TaskTreeNode(new Task(Ag::Group, "Secondary prediction")));
    for (int i=0, z=msaEntity()->msa()->rowCount(); i<z; ++i)
    {
        const AminoSeqSPtr &aminoSeq = boost::shared_static_cast<AminoSeq>(msaEntity()->msa()->at(i+1)->seqEntity_);
        int astringId = aminoSeq->abstractAnonSeq()->id();
        if (!queuedAstringIds_.contains(astringId) && aminoSeq->abstractAnonSeq()->q3().isEmpty())
        {
            ITask *task = new PredictSecondaryTask(adoc,
                                                   astringId,
                                                   aminoSeq->abstractAnonSeq()->seq_.toBioString(),
                                                   predictSecondaryDialog_->psiBlastOptions(),
                                                   aminoSeq->name());
            task->setMaxThreads(predictSecondaryDialog_->nThreads());
            group->appendChild(new TaskTreeNode(task));

            taskIdAstringIdHash_.insert(task->id(), astringId);
            queuedAstringIds_ << astringId;
        }
    }

    // No secondary structure predictions remaining
    if (group->childCount() == 0)
        return;

    taskManager()->enqueue(group.take());

    predictSecondaryStructureAction_->setEnabled(false);
}

void AminoMsaWindow::onConsensusActionToggled(bool checked)
{
    if (checked)
        msaConsensusModel_->setLiveMsaCharCountDistribution(liveMsaCharCountDistribution());
    else
        msaConsensusModel_->setLiveMsaCharCountDistribution(nullptr);
    ui_->msaView->setBottomMarginWidgetVisible(checked);
}

void AminoMsaWindow::onConsensusThresholdsActionTriggered()
{
    ConsensusOptionsDialog dialog(this);
    dialog.setThresholds(msaConsensusModel_->thresholds());
    if (!dialog.exec())
        return;
    msaConsensusModel_->setThresholds(dialog.thresholds());
    ui_->msaView->updateMarginWidgetGeometries();
}

void AminoMsaWindow::onEditConsensusGroupsActionTriggered()
{
    ConsensusGroupsDialog dialog(this);
    dialog.setUseDefaultGroups(!useCustomConsensusGroups_);
    dialog.setBioSymbolGroup(customConsensusGroup_);
    if (!dialog.exec())
        return;

    useCustomConsensusGroups_ = !dialog.useDefaultGroups();
    customConsensusGroup_ = dialog.bioSymbolGroup();

    updateConsensusModel();
}

/**
  * @param task [ITask *]
  */
void AminoMsaWindow::onTaskAboutToStart(ITask *task)
{
    ASSERT(task != nullptr);
    if (!taskIdAstringIdHash_.contains(task->id()))
        return;

    // Begin watching this task for when it completes either successfully or in error
    connect(task, SIGNAL(done(ITask*)), SLOT(onTaskDone(ITask*)));
    connect(task, SIGNAL(error(ITask*)), SLOT(onTaskError(ITask*)));
}

/**
  * @param task [ITask *]
  */
void AminoMsaWindow::onTaskDone(ITask *task)
{
    ASSERT(task != nullptr);
    ASSERT(taskIdAstringIdHash_.contains(task->id()));
    disconnect(task, SIGNAL(done(ITask*)), this, SLOT(onTaskDone(ITask*)));
    disconnect(task, SIGNAL(error(ITask*)), this, SLOT(onTaskError(ITask*)));

    // Note that this method does not update the Astring q3. Rather that is performed by the PredictSecondaryTask
    // itself. The purpose of this method is to simply repaint the relevant rows that now have secondary structure
    // prediction data.
    int astringId = taskIdAstringIdHash_.take(task->id());
    ASSERT(queuedAstringIds_.contains(astringId));
    queuedAstringIds_.remove(astringId);

    // Walk through all the amino seqs in this msa and repaint those rows
    ObservableMsa *msa = this->msa();
    ASSERT(msa != nullptr);
    for (int i=1, z=msa->rowCount(); i<=z; ++i)
    {
        const AminoSeqSPtr &aminoSeq = boost::static_pointer_cast<AminoSeq>(msa->at(i)->seqEntity_);
        if (aminoSeq->abstractAnonSeq()->id() == astringId)
        {
            ui_->msaView->repaintRow(i);
            break;
        }
    }

    if (taskIdAstringIdHash_.isEmpty())
        predictSecondaryStructureAction_->setEnabled(true);
}

/**
  * @param task [ITask *]
  */
void AminoMsaWindow::onTaskError(ITask *task)
{
    ASSERT(task != nullptr);
    ASSERT(taskIdAstringIdHash_.contains(task->id()));
    disconnect(task, SIGNAL(done(ITask*)), this, SLOT(onTaskDone(ITask*)));
    disconnect(task, SIGNAL(error(ITask*)), this, SLOT(onTaskError(ITask*)));

    // Simply update the hash associating task ids with their astring id counterparts.
    int astringId = taskIdAstringIdHash_.take(task->id());
    ASSERT(queuedAstringIds_.contains(astringId));
    queuedAstringIds_.remove(astringId);

    if (taskIdAstringIdHash_.isEmpty())
        predictSecondaryStructureAction_->setEnabled(true);
}

void AminoMsaWindow::updateConsensusLabelFont()
{
    // TODO: Consolidate this method and the MsaTableView::resizeFont functionality!

    // When passing the number of pixels to use for the calculation, 2 less pixels are used in the calculation because
    // those characters with descenders (e.g. 'j', 'g', 'q') we're being partially obscured by the next line without
    // this change. Perhaps a better alternative would be to somehow make the text render without first painting its
    // whole background rectangle. In other words, paint the entire background of the widget first, and then repaint all
    // cells by only drawing their text. Not sure how this would work in conjunction with selection.
    IRenderEngine *renderEngine = consensusView_->renderEngine();
    if (renderEngine == nullptr)
        return;

    double charHeight = renderEngine->abstractTextRenderer()->height();
    QFont font = consensusLabelTableView_->font();
    font.setItalic(true);
    int nPixels = qMax(1., charHeight - 2);
    int pointSize = ::fitPointSizeFromPixels(font.family(), nPixels, logicalDpiY());
    font.setPointSize(pointSize);
    consensusLabelTableView_->setFont(font);
    consensusLabelTableView_->verticalHeader()->setDefaultSectionSize(charHeight);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Maps a column in the original column adapter through all filters to that displayed in the table view (as returned
  * by MsaSubseqTableModel.
  *
  * @param adapterColumn [int]
  * @returns int
  */
int AminoMsaWindow::mapFromColumnAdapter(int adapterColumn) const
{
    ASSERT(filterColumnAdapter_ != nullptr);
    ASSERT(msaSubseqTableModel_ != nullptr);

    int filterColumn = filterColumnAdapter_->mapFromSource(adapterColumn);
    return msaSubseqTableModel_->mapFromColumnAdapter(filterColumn);
}

/**
  */
void AminoMsaWindow::setupColorProviders()
{
    // Basic: simply black foreground on white text - the default positional color provider state
    QAction *plainColorAction = ui_->menu_Visualizations->addAction("Plain");
    plainColorAction->setCheckable(true);
    connect(plainColorAction, SIGNAL(triggered()), SLOT(onBasicColorActionTriggered()));

    // Clustal: rich colors dynamically based on the actual amino acid composition of each column
    QAction *clustalColorAction = ui_->menu_Visualizations->addAction("Clustal");
    clustalColorAction->setCheckable(true);
    connect(clustalColorAction, SIGNAL(toggled(bool)), SLOT(onClustalColorActionToggled(bool)));

    // Secondary structure [Predicted]
    QAction *secondaryColorAction = ui_->menu_Visualizations->addAction("Secondary Structure");
    secondaryColorAction->setCheckable(true);
    connect(secondaryColorAction, SIGNAL(triggered()), SLOT(onSecondaryColorActionTriggered()));

    // Zappo: phsicochemical properties
    QAction *zappoColorAction = ui_->menu_Visualizations->addAction("Zappo");
    zappoColorAction->setCheckable(true);
    connect(zappoColorAction, SIGNAL(triggered()), SLOT(onZappaColorActionTriggered()));

    // Taylor
    QAction *taylorColorAction = ui_->menu_Visualizations->addAction("Taylor");
    taylorColorAction->setCheckable(true);
    connect(taylorColorAction, SIGNAL(triggered()), SLOT(onTaylorColorActionTriggered()));

    // Hydrophobicity
    QAction *hydroColorAction = ui_->menu_Visualizations->addAction("Hydrophobicity");
    hydroColorAction->setCheckable(true);
    connect(hydroColorAction, SIGNAL(triggered()), SLOT(onHydroColorActionTriggered()));

    // Helix propensity
    QAction *helixColorAction = ui_->menu_Visualizations->addAction("Helix Propensity");
    helixColorAction->setCheckable(true);
    connect(helixColorAction, SIGNAL(triggered()), SLOT(onHelixColorActionTriggered()));

    // Strand propensity
    QAction *strandColorAction = ui_->menu_Visualizations->addAction("Strand propensity");
    strandColorAction->setCheckable(true);
    connect(strandColorAction, SIGNAL(triggered()), SLOT(onStrandColorActionTriggered()));

    // Turn propensity
    QAction *turnColorAction = ui_->menu_Visualizations->addAction("Turn Propensity");
    turnColorAction->setCheckable(true);
    connect(turnColorAction, SIGNAL(triggered()), SLOT(onTurnColorActionTriggered()));

    // Buried index
    QAction *buriedColorAction = ui_->menu_Visualizations->addAction("Buried Index");
    buriedColorAction->setCheckable(true);
    connect(buriedColorAction, SIGNAL(triggered()), SLOT(onBuriedColorActionTriggered()));



    defaultColorProviderAction_ = clustalColorAction;

    colorProvidersGroup_ = new QActionGroup(this);
    colorProvidersGroup_->addAction(plainColorAction);
    colorProvidersGroup_->addAction(clustalColorAction);
    colorProvidersGroup_->addAction(secondaryColorAction);
    colorProvidersGroup_->addAction(zappoColorAction);
    colorProvidersGroup_->addAction(taylorColorAction);
    colorProvidersGroup_->addAction(hydroColorAction);
    colorProvidersGroup_->addAction(helixColorAction);
    colorProvidersGroup_->addAction(strandColorAction);
    colorProvidersGroup_->addAction(turnColorAction);
    colorProvidersGroup_->addAction(buriedColorAction);

    // Automatically assign up to 10 shortcuts
    QList<QAction *> actions = colorProvidersGroup_->actions();
    for (int i=0, z= actions.size(); i<z && i<10; ++i)
        actions.at(i)->setShortcut(QKeySequence(QString("Ctrl+%1").arg(i)));
}

void AminoMsaWindow::updateConsensusModel()
{
    if (useCustomConsensusGroups_)
        msaConsensusModel_->setConsensusSymbolGroupPrototype(customConsensusGroup_);
    else
        msaConsensusModel_->setConsensusSymbolGroupPrototype(constants::CommonBioSymbolGroups::defaultConsensusSymbolGroup());
}
