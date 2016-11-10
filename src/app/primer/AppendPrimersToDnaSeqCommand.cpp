/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AppendPrimersToDnaSeqCommand.h"
#include "DnaSeqPrimerVectorMutator.h"
#include "../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
AppendPrimersToDnaSeqCommand::AppendPrimersToDnaSeqCommand(const DnaSeqSPtr &dnaSeq, const PrimerVector &primers, DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator, QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      dnaSeq_(dnaSeq),
      primers_(primers),
      dnaSeqPrimerVectorMutator_(dnaSeqPrimerVectorMutator)
{
    ASSERT(dnaSeq != nullptr);
    ASSERT(primers.size() > 0);
    ASSERT(dnaSeqPrimerVectorMutator != nullptr);

    setText(QString("Adding %1 primer(s) to %2").arg(primers_.size()).arg(dnaSeq->name()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void AppendPrimersToDnaSeqCommand::redo()
{
    appendRange_ = dnaSeqPrimerVectorMutator_->appendPrimers(dnaSeq_, primers_);
}

void AppendPrimersToDnaSeqCommand::undo()
{
    dnaSeqPrimerVectorMutator_->removePrimers(dnaSeq_, appendRange_);
}
