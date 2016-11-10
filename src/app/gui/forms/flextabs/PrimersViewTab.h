/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERSVIEWTAB_H
#define PRIMERSVIEWTAB_H

#include <QtCore/QModelIndexList>
#include <QtGui/QWidget>
#include "../../../core/Entities/DnaSeq.h"

namespace Ui {
    class PrimersViewTab;
}

class QItemSelection;
class QTableView;
class QUndoStack;

class DnaSeqCommandPrimerMutator;
class DnaSeqPrimerModel;
class DnaSeqPrimerVectorMutator;
class FilterColumnProxyModel;

class PrimersViewTab : public QWidget
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit PrimersViewTab(QUndoStack *undoStack, QWidget *parent = 0);
    ~PrimersViewTab();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QTableView *primersTableView() const;
    DnaSeqPrimerModel *dnaSeqPrimerModel() const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setDnaSeq(const DnaSeqSPtr &dnaSeq);
    void removeSelectedPrimers();


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void updatePrimersTableViewRowAndColumnSizes();
    void startPrimerWizard(const PrimerSearchParameters *primerSearchParameters = nullptr);
    void pushAppendPrimerCommand(const Primer &primer);
    void pushAppendPrimersCommand(const PrimerVector &primers);
    void onManualPrimerInputAction();
    void onPrimerTableSelectionChanged();
    void onPrimerTableCurrentChanged(const QModelIndex &currentIndex);
    void showPrimerSearchParameters();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool eventFilter(QObject *object, QEvent *event);
    bool handlePrimersTableViewKeyPress(QKeyEvent *keyEvent);
    void copySelectionToClipboard();
    QVector<ClosedIntRange> getSelectedPrimerRowRanges() const;
    QModelIndexList extractUniqueRowIndices(const QModelIndexList &indices) const;
    QModelIndexList mapIndicesToSourceModel(const QModelIndexList &proxyIndexes) const;
    QVector<int> extractRowNumbersFromModelIndices(const QModelIndexList &modelIndices) const;
    void removeContiguousPrimerRange(const ClosedIntRange &primerRange);
    void removeDiscontiguousPrimerRanges(QVector<ClosedIntRange> &primerRanges);
    int sumTotalRowsToRemove(const QVector<ClosedIntRange> &primerRanges) const;
    bool userConfirmsDelete();
    QModelIndex currentPrimerIndex() const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::PrimersViewTab *ui_;
    QUndoStack *undoStack_;
    DnaSeqSPtr dnaSeq_;

    DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator_;
    DnaSeqCommandPrimerMutator *dnaSeqCommandPrimerMutator_;
    DnaSeqPrimerModel *dnaSeqPrimerModel_;
    FilterColumnProxyModel *sortFilterDnaSeqPrimerModel_;
};

#endif // PRIMERSVIEWTAB_H
