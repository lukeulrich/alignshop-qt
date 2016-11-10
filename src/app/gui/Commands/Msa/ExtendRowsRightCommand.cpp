/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "ExtendRowsRightCommand.h"
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
ExtendRowsRightCommand::ExtendRowsRightCommand(ObservableMsa *msa,
                                               int msaColumn,
                                               const ClosedIntRange &rows,
                                               QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand), msa_(msa), msaColumn_(msaColumn), rows_(rows)
{
    ASSERT(msa != nullptr);
    setText(QString("Extend rows (%1 - %2) right to column %3").arg(rows.begin_).arg(rows.end_).arg(msaColumn));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void ExtendRowsRightCommand::redo()
{
    changePodVector_ = msa_->extendRight(msaColumn_, rows_);
}

/**
  */
void ExtendRowsRightCommand::undo()
{
    msa_->undo(changePodVector_);
}
