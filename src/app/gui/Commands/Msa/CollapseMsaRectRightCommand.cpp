/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "CollapseMsaRectRightCommand.h"
#include "../../../core/ObservableMsa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [ObservableMsa *]
  * @param msaRect [const PosiRect &]
  * @param parentCommand [QUndoCommand *]
  */
CollapseMsaRectRightCommand::CollapseMsaRectRightCommand(ObservableMsa *msa, const PosiRect &msaRect, QUndoCommand *parentCommand)
    : AbstractCollapseMsaRectCommand(msa, msaRect, parentCommand)
{
    setText(QString("Collapse right ([%1, %2], [%3, %4])")
            .arg(msaRect_.left())
            .arg(msaRect_.top())
            .arg(msaRect_.right())
            .arg(msaRect_.bottom()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void CollapseMsaRectRightCommand::redo()
{
    changePodVector_ = msa_->collapseRight(msaRect_);
}

/**
  * @returns int
  */
int CollapseMsaRectRightCommand::subId() const
{
    return Ag::eCollapseMsaRectRightCommandId;
}
