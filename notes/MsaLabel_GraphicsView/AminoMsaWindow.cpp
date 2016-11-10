/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QActionGroup>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QHeaderView>

#include "AminoMsaWindow.h"

#include "../models/ColumnAdapters/AminoSeqColumnAdapter.h"
#include "../models/ColumnAdapters/FilterColumnAdapter.h"
#include "../models/MsaSubseqTableModel.h"

#include "../../core/Entities/AbstractMsa.h"
#include "../../core/LiveInfoContentDistribution.h"
#include "../../core/LiveMsaCharCountDistribution.h"
#include "../../core/LiveSymbolString.h"
#include "../../core/Services/SymbolStringCalculator.h"
#include "../../graphics/ColorSchemes.h"
#include "../../graphics/CharColorProvider.h"
#include "../../graphics/SecondaryStructureColorProvider.h"
#include "../../graphics/SymbolColorProvider.h"
#include "../painting/gitems/LogoBarsItem.h"

#include "ui_MsaWindow.h"

#include <QtCore/QScopedPointer>
#include "../../core/Adoc.h"
#include "../../core/ObservableMsa.h"
#include "../../core/Subseq.h"
#include "../Services/TaskManager.h"
#include "../Services/Tasks/PredictSecondaryTask.h"
#include "../Services/Tasks/Task.h"
#include "../Services/Tasks/TaskTreeNode.h"


#include <QtGui/QScrollBar>
#include <QtGui/QGraphicsItemGroup>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QtGui/QFontMetrics>
#include "../../graphics/CharPixelMetrics.h"
#include "../../graphics/CharPixelMetricsF.h"
#include "../painting/IRenderEngine.h"



#include "../painting/gitems/MsaColumnItemGroup.h"
#include "../gui_misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adoc [Adoc *]
  * @param taskManager [TaskManager *]
  * @param parent [QWidget *]
  */
AminoMsaWindow::AminoMsaWindow(Adoc *adoc, TaskManager *taskManager, QWidget *parent)
    : MsaWindow(adoc, taskManager, parent),
      aminoSeqColumnAdapter_(nullptr),
      filterColumnAdapter_(nullptr),
      liveSymbolString_(nullptr),
      liveInfoContentDistribution_(nullptr),
      colorProvidersGroup_(nullptr),
      defaultColorProviderAction_(nullptr),
      predictSecondaryStructureAction_(nullptr),
      logoScene_(nullptr),
      logoBarsItem_(nullptr),
      labelScene_(nullptr),
      labelItemGroup_(nullptr)
{
    aminoSeqColumnAdapter_ = new AminoSeqColumnAdapter(this);
    aminoSeqColumnAdapter_->setUndoStack(undoStack_);

    filterColumnAdapter_ = new FilterColumnAdapter(this);
    filterColumnAdapter_->setSourceAdapter(aminoSeqColumnAdapter_);
    filterColumnAdapter_->exclude(AminoSeqColumnAdapter::eIdColumn);
    filterColumnAdapter_->exclude(AminoSeqColumnAdapter::eStartColumn);
    filterColumnAdapter_->exclude(AminoSeqColumnAdapter::eStopColumn);

    setupColorProviders();

    // Currently it is not linked to a live info content distribution, does not have a parent item, but is a child
    // of this window
    logoBarsItem_ = new LogoBarsItem(nullptr, nullptr, this);
    logoBarsItem_->setColorScheme(ColorSchemes::kLogoAminoScheme);

    logoScene_ = new QGraphicsScene(this);
    logoScene_->addItem(logoBarsItem_);

    ui_->logoGraphicsView->setScene(logoScene_);

    // -------------------------------------------------
    // Setup the predict secondary structure menu option
    predictSecondaryStructureAction_ = ui_->menu_Visualizations->addAction("Predict secondary structure");
    connect(predictSecondaryStructureAction_, SIGNAL(triggered()), SLOT(onPredictSecondaryStructureActionTriggered()));

    // Until a valid msa has been set, this action should remain disabled
    predictSecondaryStructureAction_->setEnabled(false);
}

/**
  * Subclasses should return a valid pointer here if they want too.
  */
IColumnAdapter *AminoMsaWindow::subseqEntityColumnAdapter() const
{
    return filterColumnAdapter_;
}

/**
  * @param abstractMsa [AbstractMsa *]
  */
void AminoMsaWindow::setMsaEntity(AbstractMsa *abstractMsa)
{
    ASSERT(abstractMsa->type() == eAminoMsaEntity);

    MsaWindow::setMsaEntity(abstractMsa);

    // --------------------
    // Setup the logo scene
    delete liveInfoContentDistribution_;
    liveInfoContentDistribution_ = nullptr;
    if (liveMsaCharCountDistribution() != nullptr)
    {
        liveInfoContentDistribution_ = new LiveInfoContentDistribution(liveMsaCharCountDistribution(), 20, true, this);
        logoBarsItem_->setLiveInfoContentDistribution(liveInfoContentDistribution_);

        connect(liveInfoContentDistribution_, SIGNAL(columnsInserted(ClosedIntRange)), SLOT(updateSceneRect()));
        connect(liveInfoContentDistribution_, SIGNAL(columnsRemoved(ClosedIntRange)), SLOT(updateSceneRect()));
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

    ui_->menu_Colors->setEnabled(abstractMsa != nullptr);

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
    // Setup the label scene
    delete labelScene_;
    labelScene_ = new QGraphicsScene(this);
    labelItemGroup_ = nullptr;

    // Configure the label font
    QFont font = ui_->labelGraphicsView->font();
    int pointSize = ::fitPointSizeFromPixels(font.family(),
                                             ui_->msaView->charHeight(),
                                             logicalDpiY());
    font.setPointSize(pointSize);
    ui_->labelGraphicsView->setFont(font);

    // Setup the root label group
    labelItemGroup_ = new MsaColumnItemGroup(abstractMsa->msa(),
                                             aminoSeqColumnAdapter_,
                                             AminoSeqColumnAdapter::eNameColumn,
                                             ui_->labelGraphicsView->font(),
                                             ui_->msaView->baseline(),
                                             ui_->msaView->charHeight());
    // During a partial scroll, there is space to display at least one more item. Therefore, we add one to the number
    // of items calculated to fit in the viewport.
    int nRows = qMin(msaEntity()->msa()->rowCount(), ui_->msaView->viewport()->height() / labelItemGroup_->rowHeight() + 1);
    labelItemGroup_->setRows(nRows);
    labelScene_->addItem(labelItemGroup_);

    ui_->labelGraphicsView->setScene(labelScene_);

    // Always keep a minimum scene rect that is at least double the height of the viewport - this is to permit properly
    // aligning labels for very small alignments.
    int minY = qMin(static_cast<int>(-ui_->msaView->charHeight() * abstractMsa->msa()->rowCount()), -ui_->labelGraphicsView->viewport()->height());
    int maxY = qMax(static_cast<int>(ui_->msaView->charHeight() * abstractMsa->msa()->rowCount()), ui_->labelGraphicsView->viewport()->height());

    // The extra ${minY} pixels is to give the scene rect enough room to properly position itself if at the very bottom
    // of the alignment. If there is not enough room, the centerOn method will only be able to position things up to a
    // point (because it is hitting the border of the scene rect). If this happens the labels will no longer coincide
    // with the corresponding alignment section.
    //
    // Since we are specifying potential height here and the maximum zoom is 1600%, I give it 17x
    ui_->labelGraphicsView->setSceneRect(0, minY, 300, maxY - (16. + 1. * minY));

    // Position the view at the top left
    ui_->labelGraphicsView->centerOn(0, ui_->labelGraphicsView->viewport()->height() / 2.);

    // Indexing in this case is not likely be helpful since all the items are constantly moving
    labelScene_->setItemIndexMethod(QGraphicsScene::NoIndex);

    // Hook up signals so that we can keep in sync with the alignment view
    connect(ui_->msaView, SIGNAL(zoomChanged(double)), SLOT(onZoomChanged()));
    connect(ui_->msaView->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), SLOT(onRangeChanged(int,int)));
    connect(ui_->msaView->verticalScrollBar(), SIGNAL(valueChanged(int)), ui_->labelGraphicsView->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui_->msaView->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(onMsaViewVerticalScroll()));
    connect(ui_->labelGraphicsView, SIGNAL(viewportResized(QSize,QSize)), SLOT(onLabelViewportResized(QSize)));

    ui_->labelGraphicsView->horizontalScrollBar()->setRange(0, 0);
    // Note: we add one row's worth more to compensate for the fact that the label view is not permitted to have a
    // horizontal scrollbar (or vertial one either, but that's irrelevant here).
    ui_->labelGraphicsView->verticalScrollBar()->setRange(0, ui_->msaView->verticalScrollBar()->maximum() + ui_->msaView->charHeight());

    // Synchronize the scrolling in the label view with the alignment view
    connect(ui_->labelGraphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), ui_->msaView->verticalScrollBar(), SLOT(setValue(int)));
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
        liveSymbolString_ = new LiveSymbolString(liveMsaCharCountDistribution(),
                                                 SymbolStringCalculator(ColorSchemes::kClustalAminoSymbolGroup, ' '),
                                                 this);
        ui_->msaView->setPositionalMsaColorProvider(new SymbolColorProvider(liveSymbolString_, ColorSchemes::kClustalAminoScheme));
    }
    else
    {
        // This means that the clustal color action is no longer checked. Free the live char count distribution and
        // symbol string
        delete liveSymbolString_;
        liveSymbolString_ = nullptr;
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

/**
  */
void AminoMsaWindow::onSecondaryColorActionTriggered()
{
    ui_->msaView->setPositionalMsaColorProvider(new SecondaryStructureColorProvider(ColorSchemes::kSecondaryStructureScheme));
}

/**
  */
void AminoMsaWindow::updateSceneRect()
{
    ui_->logoGraphicsView->setSceneRect(logoBarsItem_->boundingRect());
}

/**
  * Note: does not check if already running...
  */
void AminoMsaWindow::onPredictSecondaryStructureActionTriggered()
{
    // Determine the list of amino seqs without secondary structure data and submit those
    Adoc *adoc = this->adoc();
    QScopedPointer<TaskTreeNode> group(new TaskTreeNode(new Task(Ag::Group, "Secondary prediction")));
    for (int i=0, z=msaEntity()->msa()->rowCount(); i<z; ++i)
    {
        AminoSeq *aminoSeq = static_cast<AminoSeq *>(msaEntity()->msa()->at(i+1)->seqEntity_);
        if (aminoSeq->abstractAnonSeq()->q3().isEmpty())
        {
            ITask *task = new PredictSecondaryTask(adoc,
                                                   aminoSeq->abstractAnonSeq()->id(),
                                                   aminoSeq->abstractAnonSeq()->seq_.asByteArray(),
                                                   aminoSeq->name());
            task->setMaxThreads(1);
            group->appendChild(new TaskTreeNode(task));
        }
    }

    // No secondary structure predictions remaining
    if (group->childCount() == 0)
        return;

    taskManager()->enqueue(group.take());
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
    QAction *basicColorAction = ui_->menu_Colors->addAction("Plain");
    basicColorAction->setCheckable(true);
    connect(basicColorAction, SIGNAL(triggered()), SLOT(onBasicColorActionTriggered()));

    // Clustal: rich colors dynamically based on the actual amino acid composition of each column
    QAction *clustalColorAction = ui_->menu_Colors->addAction("Clustal");
    clustalColorAction->setCheckable(true);
    connect(clustalColorAction, SIGNAL(toggled(bool)), SLOT(onClustalColorActionToggled(bool)));

    // Zappo: phsicochemical properties
    QAction *zappoColorAction = ui_->menu_Colors->addAction("Zappo");
    zappoColorAction->setCheckable(true);
    connect(zappoColorAction, SIGNAL(triggered()), SLOT(onZappaColorActionTriggered()));

    // Taylor
    QAction *taylorColorAction = ui_->menu_Colors->addAction("Taylor");
    taylorColorAction->setCheckable(true);
    connect(taylorColorAction, SIGNAL(triggered()), SLOT(onTaylorColorActionTriggered()));

    // Hydrophobicity
    QAction *hydroColorAction = ui_->menu_Colors->addAction("Hydrophobicity");
    hydroColorAction->setCheckable(true);
    connect(hydroColorAction, SIGNAL(triggered()), SLOT(onHydroColorActionTriggered()));

    // Helix propensity
    QAction *helixColorAction = ui_->menu_Colors->addAction("Helix Propensity");
    helixColorAction->setCheckable(true);
    connect(helixColorAction, SIGNAL(triggered()), SLOT(onHelixColorActionTriggered()));

    // Strand propensity
    QAction *strandColorAction = ui_->menu_Colors->addAction("Strand propensity");
    strandColorAction->setCheckable(true);
    connect(strandColorAction, SIGNAL(triggered()), SLOT(onStrandColorActionTriggered()));

    // Turn propensity
    QAction *turnColorAction = ui_->menu_Colors->addAction("Turn Propensity");
    turnColorAction->setCheckable(true);
    connect(turnColorAction, SIGNAL(triggered()), SLOT(onTurnColorActionTriggered()));

    // Secondary structure
    QAction *secondaryColorAction = ui_->menu_Colors->addAction("Secondary Structure");
    secondaryColorAction->setCheckable(true);
    connect(secondaryColorAction, SIGNAL(triggered()), SLOT(onSecondaryColorActionTriggered()));


    defaultColorProviderAction_ = clustalColorAction;

    colorProvidersGroup_ = new QActionGroup(this);
    colorProvidersGroup_->addAction(basicColorAction);
    colorProvidersGroup_->addAction(clustalColorAction);
    colorProvidersGroup_->addAction(zappoColorAction);
    colorProvidersGroup_->addAction(taylorColorAction);
    colorProvidersGroup_->addAction(hydroColorAction);
    colorProvidersGroup_->addAction(helixColorAction);
    colorProvidersGroup_->addAction(strandColorAction);
    colorProvidersGroup_->addAction(turnColorAction);
    colorProvidersGroup_->addAction(secondaryColorAction);

    // By default, none of the color providers are initially checked
}


/**
  * @param size [const QSize &]
  */
void AminoMsaWindow::onLabelViewportResized(const QSize &size)
{
    if (labelItemGroup_ == nullptr)
        return;

    // Since the viewport is now a different size, we need fewer/more items in the scene to correspond to their labels
    int nRows = qMin(msaEntity()->msa()->rowCount(), size.height() / labelItemGroup_->rowHeight() + 1);
    labelItemGroup_->setRows(nRows);

    ui_->labelGraphicsView->verticalScrollBar()->setRange(0, ui_->msaView->verticalScrollBar()->maximum());
}

/**
  */
void AminoMsaWindow::onMsaViewVerticalScroll()
{
    int vertScrollPos = ui_->msaView->verticalScrollBar()->value();
    int offset = vertScrollPos % qRound(ui_->msaView->charHeight());
    labelItemGroup_->setPos(0, vertScrollPos - offset);
}

/**
  */
void AminoMsaWindow::onZoomChanged()
{
    // Configure the label font
    QFont font = ui_->labelGraphicsView->font();
    int pointSize = ::fitPointSizeFromPixels(font.family(),
                                             ui_->msaView->charHeight(),
                                             logicalDpiY());
    font.setPointSize(pointSize);
    ui_->labelGraphicsView->setFont(font);

    // There is no point in having more rows than there are sequences in the Msa, so cap it if necessary
    int nRows = ui_->labelGraphicsView->viewport()->height() / ui_->msaView->charHeight() + 1;
    ASSERT(nRows > 0);
    labelItemGroup_->setRows(qMin(msaEntity()->msa()->rowCount(), nRows));

    labelItemGroup_->setFont(ui_->labelGraphicsView->font());
    labelItemGroup_->setBaseline(ui_->msaView->baseline());
    labelItemGroup_->setRowHeight(ui_->msaView->charHeight());

    ui_->labelGraphicsView->verticalScrollBar()->setRange(0, ui_->msaView->verticalScrollBar()->maximum());
    ui_->labelGraphicsView->verticalScrollBar()->setValue(ui_->msaView->verticalScrollBar()->value());
}

void AminoMsaWindow::onRangeChanged(int min, int max)
{
    ui_->labelGraphicsView->verticalScrollBar()->setRange(min, max);
}
