/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "CollapseMsaRectLeftCommand.h"
#include "../../../core/ObservableMsa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [ObservableMsa *]
  * @param msaRect [const PosiRect &]
  * @param parentCommand [QUndoCommand *]
  */
CollapseMsaRectLeftCommand::CollapseMsaRectLeftCommand(ObservableMsa *msa, const PosiRect &msaRect, QUndoCommand *parentCommand)
    : AbstractCollapseMsaRectCommand(msa, msaRect, parentCommand)
{
    setText(QString("Collapse left ([%1, %2], [%3, %4])")
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
void CollapseMsaRectLeftCommand::redo()
{
    changePodVector_ = msa_->collapseLeft(msaRect_);
}

/**
  * @returns int
  */
int CollapseMsaRectLeftCommand::subId() const
{
    return Ag::eCollapseMsaRectLeftCommandId;
}
