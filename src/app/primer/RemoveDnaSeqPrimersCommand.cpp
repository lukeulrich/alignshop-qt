/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "RemoveDnaSeqPrimersCommand.h"
#include "DnaSeqPrimerVectorMutator.h"
#include "../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
RemoveDnaSeqPrimersCommand::RemoveDnaSeqPrimersCommand(const DnaSeqSPtr &dnaSeq, const ClosedIntRange &removeRange, DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator, QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      dnaSeq_(dnaSeq),
      removeRange_(removeRange),
      dnaSeqPrimerVectorMutator_(dnaSeqPrimerVectorMutator)
{
    ASSERT(dnaSeq != nullptr);
    ASSERT(removeRange.length() > 0);
    ASSERT(dnaSeqPrimerVectorMutator != nullptr);

    setText(QString("Remove %1 primers from %2").arg(removeRange.length()).arg(dnaSeq->name()));
    removedPrimers_ = dnaSeq_->primers_.mid(removeRange_.begin_, removeRange_.length());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void RemoveDnaSeqPrimersCommand::redo()
{
    dnaSeqPrimerVectorMutator_->removePrimers(dnaSeq_, removeRange_);
}

void RemoveDnaSeqPrimersCommand::undo()
{
    dnaSeqPrimerVectorMutator_->insertPrimers(dnaSeq_, removeRange_.begin_, removedPrimers_);
}
