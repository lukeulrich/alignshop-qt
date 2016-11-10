/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QActionGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QScrollBar>

#include "DnaMsaWindow.h"

#include "../models/ColumnAdapters/DnaSeqColumnAdapter.h"
#include "../models/ColumnAdapters/FilterColumnAdapter.h"
#include "../models/MsaSubseqTableModel.h"

#include "../../core/Entities/AbstractMsa.h"
#include "../../core/LiveInfoContentDistribution.h"
#include "../../core/LiveMsaCharCountDistribution.h"

#include "../../graphics/ColorSchemes.h"
#include "../../graphics/CharColorProvider.h"

#include "../painting/gitems/LogoBarsItem.h"
#include "../painting/gitems/LogoItem.h"

#include "ui_MsaWindow.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adoc [Adoc *]
  * @param taskManager [TaskManager *]
  * @param parent [QWidget *]
  */
DnaMsaWindow::DnaMsaWindow(Adoc *adoc, TaskManager *taskManager, QWidget *parent)
    : MsaWindow(adoc, taskManager, parent),
      dnaSeqColumnAdapter_(nullptr),
      filterColumnAdapter_(nullptr),
      liveInfoContentDistribution_(nullptr)
{
    setWindowIcon(QIcon(":aliases/images/icons/dna-msa"));

    dnaSeqColumnAdapter_ = new DnaSeqColumnAdapter(this);
    dnaSeqColumnAdapter_->setUndoStack(undoStack_);

    filterColumnAdapter_ = new FilterColumnAdapter(this);
    filterColumnAdapter_->setSourceAdapter(dnaSeqColumnAdapter_);
    filterColumnAdapter_->exclude(DnaSeqColumnAdapter::eIdColumn);
    filterColumnAdapter_->exclude(DnaSeqColumnAdapter::eStartColumn);
    filterColumnAdapter_->exclude(DnaSeqColumnAdapter::eStopColumn);

    setupColorProviders();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns IColumnAdapter *
  */
IColumnAdapter *DnaMsaWindow::subseqEntityColumnAdapter() const
{
    return filterColumnAdapter_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
LiveInfoContentDistribution *DnaMsaWindow::liveInfoContentDistribution() const
{
    return liveInfoContentDistribution_;
}

/**
  * @param abstractMsa [AbstractMsa *]
  */
void DnaMsaWindow::setMsaEntityImpl(const AbstractMsaSPtr &abstractMsa)
{
    ASSERT(abstractMsa->type() == eDnaMsaEntity);

    // --------------------
    // Setup the logo scene
    delete liveInfoContentDistribution_;
    liveInfoContentDistribution_ = nullptr;
    if (liveMsaCharCountDistribution() != nullptr)
    {
        liveInfoContentDistribution_ = new LiveInfoContentDistribution(liveMsaCharCountDistribution(), 4, true, this);
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
    ui_->subseqTableView->horizontalHeader()->hideSection(mapFromColumnAdapter(DnaSeqColumnAdapter::eTypeColumn));
    ui_->subseqTableView->horizontalHeader()->hideSection(mapFromColumnAdapter(DnaSeqColumnAdapter::eNotesColumn));
    ui_->subseqTableView->horizontalHeader()->hideSection(mapFromColumnAdapter(DnaSeqColumnAdapter::eSequenceColumn));

    // Move the name column to be the first column in the list
    ui_->subseqTableView->horizontalHeader()->moveSection(mapFromColumnAdapter(DnaSeqColumnAdapter::eNameColumn), 0);
    ui_->subseqTableView->resizeColumnToContents(mapFromColumnAdapter(DnaSeqColumnAdapter::eSourceColumn));

    ui_->menu_Visualizations->setEnabled(abstractMsa != nullptr);

    // Trigger the coloring for the currently selected color scheme
    QAction *currentColorAction = colorProvidersGroup_->checkedAction();
    if (currentColorAction)
        currentColorAction->activate(QAction::Trigger);
    else
        // Or use the default color provider if none is selected
        defaultColorProviderAction_->activate(QAction::Trigger);


    // -------------------------------------------------------
    // Choose the appropriate column we want to show in the label view
    ui_->labelView->setColumn(mapFromColumnAdapter(DnaSeqColumnAdapter::eNameColumn));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void DnaMsaWindow::onBasicColorActionTriggered()
{
    // By setting the color provider to null, it will use the default renderer which is simply black on white.
    ui_->msaView->setPositionalMsaColorProvider(nullptr);
}

/**
  */
void DnaMsaWindow::onClustalColorActionToggled()
{
    // By setting the color provider to null, it will use the default renderer which is simply black on white.
    ui_->msaView->setPositionalMsaColorProvider(new CharColorProvider(ColorSchemes::kClustalDnaScheme));
}



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param adapterColumn [int]
  * @returns int
  */
int DnaMsaWindow::mapFromColumnAdapter(int adapterColumn) const
{
    ASSERT(filterColumnAdapter_ != nullptr);
    ASSERT(msaSubseqTableModel_ != nullptr);

    int filterColumn = filterColumnAdapter_->mapFromSource(adapterColumn);
    return msaSubseqTableModel_->mapFromColumnAdapter(filterColumn);
}

/**
  */
void DnaMsaWindow::setupColorProviders()
{
    // Basic: simply black foreground on white text - the default positional color provider state
    QAction *basicColorAction = ui_->menu_Visualizations->addAction("Plain");
    basicColorAction->setCheckable(true);
    connect(basicColorAction, SIGNAL(triggered()), SLOT(onBasicColorActionTriggered()));

    // Clustal: rich colors dynamically based on the actual amino acid composition of each column
    QAction *clustalColorAction = ui_->menu_Visualizations->addAction("Clustal");
    clustalColorAction->setCheckable(true);
    connect(clustalColorAction, SIGNAL(triggered()), SLOT(onClustalColorActionToggled()));

    defaultColorProviderAction_ = clustalColorAction;

    colorProvidersGroup_ = new QActionGroup(this);
    colorProvidersGroup_->addAction(basicColorAction);
    colorProvidersGroup_->addAction(clustalColorAction);

    // Automatically assign up to 10 shortcuts
    QList<QAction *> actions = colorProvidersGroup_->actions();
    for (int i=0, z= actions.size(); i<z && i<10; ++i)
        actions.at(i)->setShortcut(QKeySequence(QString("Ctrl+%1").arg(i)));
}
