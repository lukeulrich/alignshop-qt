/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "RemoveRowsCommand.h"
#include "../../../core/Entities/AbstractMsa.h"
#include "../../../core/ObservableMsa.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [ObservableMsa *]
  * @param rows [const ClosedIntRange &]
  * @param parentCommand [QUndoCommand *]
  */
RemoveRowsCommand::RemoveRowsCommand(const AbstractMsaSPtr &msaEntity, const ClosedIntRange &rows, QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand), msaEntity_(msaEntity), rows_(rows)
{
    ASSERT(msaEntity->msa() != nullptr);
    if (rows_.length() > 1)
        setText(QString("Remove rows: %1 - %2").arg(rows.begin_).arg(rows.end_));
    else
        setText(QString("Remove row: %1").arg(rows.begin_));
}

/**
  * When destroyed, we pass ownership of any Subseqs back to the MsaEntity via the addDeadSubseqs method. These Subseqs
  * are permanently lost, yet still loaded in the repository and thus it is now the Entities responsibility to properly
  * unload these.
  */
RemoveRowsCommand::~RemoveRowsCommand()
{
    msaEntity_->addDeadSubseqs(removedSubseqs_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * When this command is executed, ownership of the removed subseqs is temporarily passed to this instance.
  */
void RemoveRowsCommand::redo()
{
    removedSubseqs_ = msaEntity_->msa()->takeRows(rows_);
}

/**
  * Undo passes ownership of the Subseqs back to the Msa.
  */
void RemoveRowsCommand::undo()
{
    msaEntity_->msa()->insertRows(rows_.begin_, removedSubseqs_);
    removedSubseqs_.clear();
}
