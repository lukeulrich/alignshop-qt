/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "TrimRowsLeftCommand.h"

#include "../../../core/ObservableMsa.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param msa [ObservableMsa *]
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @param parentCommand [QUndoCommand *]
  */
TrimRowsLeftCommand::TrimRowsLeftCommand(ObservableMsa *msa,
                                         int msaColumn,
                                         const ClosedIntRange &rows,
                                         QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand), msa_(msa), msaColumn_(msaColumn), rows_(rows)
{
    ASSERT(msa != nullptr);
    setText(QString("Trim rows (%1 - %2) left to column %3").arg(rows.begin_).arg(rows.end_).arg(msaColumn));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void TrimRowsLeftCommand::redo()
{
    changePodVector_ = msa_->trimLeft(msaColumn_, rows_);
}

/**
  */
void TrimRowsLeftCommand::undo()
{
    msa_->undo(changePodVector_);
}
