/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "LevelRowsRightCommand.h"
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
LevelRowsRightCommand::LevelRowsRightCommand(ObservableMsa *msa,
                                             int msaColumn,
                                             const ClosedIntRange &rows,
                                             QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand), msa_(msa), msaColumn_(msaColumn), rows_(rows)
{
    ASSERT(msa != nullptr);
    setText(QString("Level rows (%1 - %2) right to column %3").arg(rows.begin_).arg(rows.end_).arg(msaColumn));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void LevelRowsRightCommand::redo()
{
    changePodVector_ = msa_->levelRight(msaColumn_, rows_);
}

/**
  */
void LevelRowsRightCommand::undo()
{
    msa_->undo(changePodVector_);
}
