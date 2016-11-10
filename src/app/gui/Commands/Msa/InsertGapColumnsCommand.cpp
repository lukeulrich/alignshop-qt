/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "InsertGapColumnsCommand.h"
#include "../../../core/ObservableMsa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [ObservableMsa *]
  * @param column [int]
  * @param count [int]
  * @param parentCommand [QUndoCommand *]
  */
InsertGapColumnsCommand::InsertGapColumnsCommand(ObservableMsa *msa, int column, int count, QUndoCommand *parentCommand)
    : AbstractSkipFirstRedoCommand(parentCommand), msa_(msa), column_(column), count_(count)
{
    ASSERT(msa_);
    setText(QString("Insert gap columns (%1 - %2)").arg(column).arg(column + count - 1));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void InsertGapColumnsCommand::redoDelegate()
{
    msa_->insertGapColumns(column_, count_);
}

/**
  */
void InsertGapColumnsCommand::undo()
{
    msa_->removeGapColumns(ClosedIntRange(column_, column_ + count_ - 1));
}
