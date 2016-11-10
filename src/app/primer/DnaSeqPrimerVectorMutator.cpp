/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DnaSeqPrimerVectorMutator.h"
#include "../core/Entities/EntityFlags.h"
#include "../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
DnaSeqPrimerVectorMutator::DnaSeqPrimerVectorMutator(QObject *parent)
    : QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
ClosedIntRange DnaSeqPrimerVectorMutator::appendPrimers(DnaSeqSPtr &dnaSeq, const PrimerVector &primerVector) const
{
    if (!dnaSeq)
        return ClosedIntRange();

    if (primerVector.isEmpty())
        return ClosedIntRange();

    int startRow = dnaSeq->primers_.size();
    int endRow = startRow + primerVector.size() - 1;

    emit primersAboutToBeAppended(dnaSeq, startRow, endRow);
    dnaSeq->primers_ << primerVector;
    dnaSeq->setDirty(Ag::ePrimersFlag, true);
    emit primersAppended(dnaSeq, startRow, endRow);

    return ClosedIntRange(startRow, endRow);
}

void DnaSeqPrimerVectorMutator::insertPrimers(DnaSeqSPtr &dnaSeq, int row, const PrimerVector &primerVector) const
{
    if (!dnaSeq)
        return;

    ASSERT(row >= 0 && row <= dnaSeq->primers_.size());

    if (primerVector.isEmpty())
        return;

    int startRow = row;
    int endRow = startRow + primerVector.size() - 1;

    emit primersAboutToBeInserted(dnaSeq, startRow, endRow);
    for (int i=startRow, j=0; i<= endRow; ++i, ++j)
        dnaSeq->primers_.insert(i, primerVector.at(j));
    dnaSeq->setDirty(Ag::ePrimersFlag, true);
    emit primersInserted(dnaSeq, startRow, endRow);
}

void DnaSeqPrimerVectorMutator::removePrimers(DnaSeqSPtr &dnaSeq, const ClosedIntRange &primerRowRange) const
{
    if (!dnaSeq)
        return;

    ASSERT(!primerRowRange.isEmpty());
    ASSERT(primerRowRange.begin_ >= 0 && primerRowRange.end_ < dnaSeq->primers_.size());

    emit primersAboutToBeRemoved(dnaSeq, primerRowRange.begin_, primerRowRange.end_);
    dnaSeq->primers_.remove(primerRowRange.begin_, primerRowRange.length());
    dnaSeq->setDirty(Ag::ePrimersFlag, true);
    emit primersRemoved(dnaSeq, primerRowRange.begin_, primerRowRange.end_);
}
