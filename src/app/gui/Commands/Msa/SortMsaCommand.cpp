/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SortMsaCommand.h"
#include "../../../core/ObservableMsa.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [ObservableMsa *]
  * @param subseqLessThan [ISubseqLessThan *]
  * @param sortOrder [Qt::SortOrder]
  * @param parentCommand [QUndoCommand *]
  */
SortMsaCommand::SortMsaCommand(ObservableMsa *msa,
							   ISubseqLessThan *subseqLessThan,
							   Qt::SortOrder sortOrder,
							   QUndoCommand *parentCommand)
	: QUndoCommand(parentCommand), msa_(msa), subseqLessThan_(subseqLessThan), sortOrder_(sortOrder)
{
	ASSERT(msa != nullptr);
	oldSubseqVector_ = msa_->subseqs_;
	setText(QString("Sort %1").arg((sortOrder_ == Qt::AscendingOrder) ? "Ascending" : "Descending"));
}

SortMsaCommand::~SortMsaCommand()
{
	delete subseqLessThan_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void SortMsaCommand::redo()
{
    msa_->sort(*subseqLessThan_, sortOrder_);
}

/**
  */
void SortMsaCommand::undo()
{
    emit msa_->rowsAboutToBeSorted();
    msa_->subseqs_ = oldSubseqVector_;
    emit msa_->rowsSorted();
}
