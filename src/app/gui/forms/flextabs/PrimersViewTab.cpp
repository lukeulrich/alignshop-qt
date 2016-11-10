/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QModelIndex>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <QtCore/QVariant>

#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTableView>
#include <QtGui/QKeyEvent>

#include "PrimersViewTab.h"
#include "ui_PrimerViewTab.h"
#include "../../Services/HeaderColumnSelector.h"
#include "../../../core/macros.h"
#include "../../../core/misc.h"
#include "../../models/FilterColumnProxyModel.h"
#include "../../wizards/PrimerCreatorWizard.h"
#include "../../../primer/AppendPrimersToDnaSeqCommand.h"
#include "../../../primer/DnaSeqPrimerModel.h"
#include "../../../primer/DnaSeqCommandPrimerMutator.h"
#include "../../../primer/DnaSeqPrimerVectorMutator.h"
#include "../../../primer/PrimerMutator.h"
#include "../../../primer/RemoveDnaSeqPrimersCommand.h"
#include "../dialogs/NewPrimerDialog.h"
#include "../dialogs/PrimerSearchParametersInfoDialog.h"

#include "../../delegates/RestrictionEnzymeAccentDelegate.h"

// ------------------------------------------------------------------------------------------------
// Forward declared methods
bool rangeBeginGreaterThan(const ClosedIntRange &a, const ClosedIntRange &b);


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
PrimersViewTab::PrimersViewTab(QUndoStack *undoStack, QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::PrimersViewTab),
    undoStack_(undoStack)
{
    ASSERT(undoStack_ != nullptr);
    ui_->setupUi(this);

    // Special setup for the toolbar (e.g. add a delete button)
    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->show();
    ui_->toolBar->addWidget(spacerWidget);
    ui_->toolBar->addAction(ui_->actionDeletePrimers);
    connect(ui_->actionDeletePrimers, SIGNAL(triggered()), SLOT(removeSelectedPrimers()));
    connect(ui_->actionSearchInfo, SIGNAL(triggered()), SLOT(showPrimerSearchParameters()));

    // Other setup
    dnaSeqPrimerVectorMutator_ = new DnaSeqPrimerVectorMutator(this);
    PrimerMutator *primerPairMutator = new PrimerMutator(this);
    dnaSeqCommandPrimerMutator_ = new DnaSeqCommandPrimerMutator(primerPairMutator, undoStack_, this);
    dnaSeqCommandPrimerMutator_->setUndoStack(undoStack_);
    dnaSeqPrimerModel_ = new DnaSeqPrimerModel(dnaSeqPrimerVectorMutator_, dnaSeqCommandPrimerMutator_, this);
    connect(dnaSeqPrimerModel_, SIGNAL(dnaSeqChanged(DnaSeqSPtr)), dnaSeqCommandPrimerMutator_, SLOT(setDnaSeq(DnaSeqSPtr)));
    connect(dnaSeqPrimerVectorMutator_, SIGNAL(primersAppended(DnaSeqSPtr,int,int)), SLOT(updatePrimersTableViewRowAndColumnSizes()));

    connect(ui_->actionPrimerWizard, SIGNAL(triggered()), SLOT(startPrimerWizard()));
    connect(ui_->actionManualPrimerInput, SIGNAL(triggered()), SLOT(onManualPrimerInputAction()));
    ui_->primersTableView->installEventFilter(this);
    new HeaderColumnSelector(ui_->primersTableView->horizontalHeader(), ui_->primersTableView);
    sortFilterDnaSeqPrimerModel_ = new FilterColumnProxyModel(this);
    sortFilterDnaSeqPrimerModel_->setDynamicSortFilter(true);
    sortFilterDnaSeqPrimerModel_->setSourceModel(dnaSeqPrimerModel_);
    ui_->primersTableView->horizontalHeader()->setMovable(true);
    ui_->primersTableView->setModel(sortFilterDnaSeqPrimerModel_);
    ui_->primersTableView->hideColumn(sortFilterDnaSeqPrimerModel_->mapFromSource(DnaSeqPrimerModel::eCoreSequenceColumn));
    ui_->primersTableView->hideColumn(sortFilterDnaSeqPrimerModel_->mapFromSource(DnaSeqPrimerModel::eRestrictionEnzymeSequenceColumn));

    RestrictionEnzymeAccentDelegate *delegate =
            new RestrictionEnzymeAccentDelegate(sortFilterDnaSeqPrimerModel_->mapFromSource(DnaSeqPrimerModel::eRestrictionEnzymeSequenceColumn), this);
    ui_->primersTableView->setItemDelegateForColumn(sortFilterDnaSeqPrimerModel_->mapFromSource(DnaSeqPrimerModel::eSequenceColumn), delegate);

    connect(ui_->primersTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onPrimerTableSelectionChanged()));

    connect(ui_->primersTableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onPrimerTableCurrentChanged(QModelIndex)));
}

PrimersViewTab::~PrimersViewTab()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
QTableView *PrimersViewTab::primersTableView() const
{
    return ui_->primersTableView;
}

DnaSeqPrimerModel *PrimersViewTab::dnaSeqPrimerModel() const
{
    return dnaSeqPrimerModel_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
void PrimersViewTab::setDnaSeq(const DnaSeqSPtr &dnaSeq)
{
    dnaSeq_ = dnaSeq;
    dnaSeqPrimerModel_->setDnaSeq(dnaSeq_);
    bool dnaSeqIsInstance = dnaSeq_;
    ui_->actionPrimerWizard->setEnabled(dnaSeqIsInstance);
    ui_->actionManualPrimerInput->setEnabled(dnaSeqIsInstance);
    QTimer::singleShot(0, this, SLOT(updatePrimersTableViewRowAndColumnSizes()));
    ui_->actionDeletePrimers->setEnabled(false);
    ui_->actionSearchInfo->setEnabled(false);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void PrimersViewTab::updatePrimersTableViewRowAndColumnSizes()
{
    ui_->primersTableView->resizeColumnsToContents();
    ui_->primersTableView->resizeRowsToContents();
}

void PrimersViewTab::startPrimerWizard(const PrimerSearchParameters *primerSearchParameters)
{
    ASSERT(dnaSeq_ != nullptr);

    PrimerCreatorWizard primerWizard(dnaSeq_->parentBioString(), primerSearchParameters, this);
    primerWizard.setInitialSearchRange(dnaSeq_->range());
    if (!primerWizard.exec())
        return;

    PrimerPairVector primerPairs = primerWizard.generatedPrimerPairs();
    if (primerPairs.isEmpty())
        return;

    PrimerVector primers;
    foreach (const PrimerPair &primerPair, primerPairs)
        primers << primerPair.forwardPrimer() << primerPair.reversePrimer();
    pushAppendPrimersCommand(primers);
}

void PrimersViewTab::pushAppendPrimerCommand(const Primer &primer)
{
    pushAppendPrimersCommand(PrimerVector() << primer);
}

void PrimersViewTab::pushAppendPrimersCommand(const PrimerVector &primers)
{
    AppendPrimersToDnaSeqCommand *appendCommand = new AppendPrimersToDnaSeqCommand(dnaSeq_, primers, dnaSeqPrimerVectorMutator_);
    undoStack_->push(appendCommand);
}

void PrimersViewTab::onManualPrimerInputAction()
{
    NewPrimerDialog newPrimerDialog(this);
    if (!newPrimerDialog.exec())
        return;

    pushAppendPrimerCommand(newPrimerDialog.primer());
}

void PrimersViewTab::onPrimerTableSelectionChanged()
{
    bool atLeastOnePrimerSelected = ui_->primersTableView->selectionModel()->selectedIndexes().size() > 0;
    ui_->actionDeletePrimers->setEnabled(atLeastOnePrimerSelected);
}

void PrimersViewTab::onPrimerTableCurrentChanged(const QModelIndex &currentIndex)
{
    ui_->actionSearchInfo->setEnabled(dnaSeqPrimerModel_->indexHasPrimerSearchParameters(currentIndex));
}

void PrimersViewTab::showPrimerSearchParameters()
{
    QModelIndex primerIndex = currentPrimerIndex();
    Primer primer = dnaSeqPrimerModel_->primerFromIndex(primerIndex);
    const PrimerSearchParameters *psp = primer.primerSearchParameters();
    ASSERT(psp != nullptr);
    PrimerSearchParametersInfoDialog dialog(this);
    dialog.setWindowTitle(QString("%1 - search parameters").arg(primer.name()));
    dialog.setPrimerSearchParameters(psp);
    if (!dialog.exec())
        return;

    startPrimerWizard(psp);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
bool PrimersViewTab::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress && object == ui_->primersTableView)
        if (handlePrimersTableViewKeyPress(static_cast<QKeyEvent *>(event)))
            return true;

    return QObject::eventFilter(object, event);
}

bool PrimersViewTab::handlePrimersTableViewKeyPress(QKeyEvent *keyEvent)
{
    if (keyEvent->matches(QKeySequence::Copy))
    {
        copySelectionToClipboard();
        keyEvent->accept();
        return true;
    }
    if (keyEvent->key() == Qt::Key_Delete)
    {
        removeSelectedPrimers();
        keyEvent->accept();
        return true;
    }

    return false;
}

void PrimersViewTab::copySelectionToClipboard()
{
    QModelIndexList selectedIndices = ui_->primersTableView->selectionModel()->selection().indexes();
    if (selectedIndices.isEmpty())
        return;

    qStableSort(selectedIndices);

    QString textForClipboard = selectedIndices.first().data().toString();
    int lastRow = selectedIndices.first().row();
    selectedIndices.removeFirst();
    foreach (const QModelIndex &index, selectedIndices)
    {
        if (index.row() == lastRow)
            textForClipboard.append("\t");
        else
            textForClipboard.append("\n");
        textForClipboard.append(index.data().toString());
        lastRow = index.row();
    }
    qApp->clipboard()->setText(textForClipboard);
}

void PrimersViewTab::removeSelectedPrimers()
{
    QVector<ClosedIntRange> selectedPrimerRanges = getSelectedPrimerRowRanges();
    if (selectedPrimerRanges.isEmpty())
        return;

    if(!userConfirmsDelete())
        return;

    if (selectedPrimerRanges.size() == 1)
        removeContiguousPrimerRange(selectedPrimerRanges.first());
    else
        removeDiscontiguousPrimerRanges(selectedPrimerRanges);
}

QVector<ClosedIntRange> PrimersViewTab::getSelectedPrimerRowRanges() const
{
    QModelIndexList proxySelectedIndices = ui_->primersTableView->selectionModel()->selectedIndexes();
    if (proxySelectedIndices.isEmpty())
        return QVector<ClosedIntRange>();
    QModelIndexList proxySelectedPrimerRows = extractUniqueRowIndices(proxySelectedIndices);
    QModelIndexList sourceSelectedPrimerRows = mapIndicesToSourceModel(proxySelectedPrimerRows);
    QVector<int> rowNumbers = extractRowNumbersFromModelIndices(sourceSelectedPrimerRows);
    return ::convertIntVectorToClosedIntRanges(rowNumbers);
}

QModelIndexList PrimersViewTab::extractUniqueRowIndices(const QModelIndexList &indices) const
{
    QModelIndexList uniqueRowIndices;
    QSet<int> observedRows;
    foreach (const QModelIndex &index, indices)
    {
        int row = index.row();
        if (observedRows.contains(row))
            continue;

        observedRows << row;
        uniqueRowIndices << index;
    }
    return uniqueRowIndices;
}

QModelIndexList PrimersViewTab::mapIndicesToSourceModel(const QModelIndexList &proxyIndices) const
{
    QModelIndexList mappedIndices;
    foreach (const QModelIndex &proxyIndex, proxyIndices)
        mappedIndices << sortFilterDnaSeqPrimerModel_->mapToSource(proxyIndex);
    return mappedIndices;
}

QVector<int> PrimersViewTab::extractRowNumbersFromModelIndices(const QModelIndexList &modelIndices) const
{
    QVector<int> rowNumbers;
    foreach (const QModelIndex &index, modelIndices)
        rowNumbers << index.row();
    return rowNumbers;
}

void PrimersViewTab::removeContiguousPrimerRange(const ClosedIntRange &primerRange)
{
    RemoveDnaSeqPrimersCommand *removeCommand = new RemoveDnaSeqPrimersCommand(dnaSeq_, primerRange, dnaSeqPrimerVectorMutator_);
    undoStack_->push(removeCommand);
}

void PrimersViewTab::removeDiscontiguousPrimerRanges(QVector<ClosedIntRange> &primerRanges)
{
    int rowsToRemove = sumTotalRowsToRemove(primerRanges);
    qStableSort(primerRanges.begin(), primerRanges.end(), rangeBeginGreaterThan);

    QUndoCommand *masterCommand = new QUndoCommand;
    masterCommand->setText(QString("Remove %1 primers from %2").arg(rowsToRemove).arg(dnaSeq_->name()));
    foreach (const ClosedIntRange &primerRange, primerRanges)
        new RemoveDnaSeqPrimersCommand(dnaSeq_, primerRange, dnaSeqPrimerVectorMutator_, masterCommand);
    undoStack_->push(masterCommand);
}

int PrimersViewTab::sumTotalRowsToRemove(const QVector<ClosedIntRange> &primerRanges) const
{
    int sum = 0;
    foreach (const ClosedIntRange &range, primerRanges)
        sum += range.length();
    return sum;
}

bool PrimersViewTab::userConfirmsDelete()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirm delete");
    msgBox.setText("Are you sure you want to delete the selected primers?");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    msgBox.button(QMessageBox::Cancel)->setText("Cancel Delete");
    msgBox.setDefaultButton(QMessageBox::Cancel);
    return msgBox.exec() == QMessageBox::Yes;
}

QModelIndex PrimersViewTab::currentPrimerIndex() const
{
    return sortFilterDnaSeqPrimerModel_->mapToSource(ui_->primersTableView->selectionModel()->currentIndex());
}

bool rangeBeginGreaterThan(const ClosedIntRange &a, const ClosedIntRange &b)
{
    return a.begin_ > b.begin_;
}
