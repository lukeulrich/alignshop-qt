/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "RemoveAllGapColumnsCommand.h"
#include "../../../core/ObservableMsa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [ObservableMsa *]
  * @param columns [const QVector<ClosedIntRange> &]
  * @param parentCommand [QUndoCommand *]
  */
RemoveAllGapColumnsCommand::RemoveAllGapColumnsCommand(ObservableMsa *msa,
                                                       const QVector<ClosedIntRange> &removedColumns,
                                                       QUndoCommand *parentCommand)
    : AbstractSkipFirstRedoCommand(parentCommand), msa_(msa), removedColumns_(removedColumns)
{
    ASSERT(msa != nullptr);
    setText("Remove all gap columns");
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void RemoveAllGapColumnsCommand::redoDelegate()
{
    msa_->removeGapColumns();
}

/**
  */
void RemoveAllGapColumnsCommand::undo()
{
    // !! Note: it is vital that we re-insert the gap columns in ascending order to preserve the proper placement of
    //    gaps.
    for (int i=0, z=removedColumns_.size(); i<z; ++i)
    {
        const ClosedIntRange &range = removedColumns_.at(i);
        msa_->insertGapColumns(range.begin_, range.length());
    }
}
