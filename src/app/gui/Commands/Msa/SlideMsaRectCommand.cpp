/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SlideMsaRectCommand.h"
#include "../../../core/ObservableMsa.h"
#include "../../widgets/AbstractMsaView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msaView [AbstractMsaView *]
  * @param msa [ObservableMsa *]
  * @param msaRect [const PosiRect &]
  * @param delta [int]
  * @param parentCommand [QUndoCommand *]
  */
SlideMsaRectCommand::SlideMsaRectCommand(AbstractMsaView *msaView,
                                         ObservableMsa *msa,
                                         const PosiRect &msaRect,
                                         int delta,
                                         QUndoCommand *parentCommand)
    : AbstractSkipFirstRedoCommand(parentCommand), msaView_(msaView), msa_(msa), msaRect_(msaRect), delta_(delta)
{
    ASSERT(msaView != nullptr);
    ASSERT(msa != nullptr);
    setText(QString("Slide rectangle [(%1, %2), (%3, %4)] %5 positions").arg(msaRect.left()).arg(msaRect.top()).arg(msaRect.right()).arg(msaRect.bottom()).arg(delta));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void SlideMsaRectCommand::redoDelegate()
{
    msa_->slideRect(msaRect_, delta_);
    PosiRect shiftedRect = msaRect_;
    shiftedRect.moveLeft(msaRect_.left() + delta_);
    msaView_->setSelection(shiftedRect);

    if (!msaView_->isPartlyVisible(shiftedRect))
        msaView_->centerOn(shiftedRect);
}

/**
  */
void SlideMsaRectCommand::undo()
{
    PosiRect shiftedRect = msaRect_;
    shiftedRect.moveLeft(msaRect_.left() + delta_);
    msa_->slideRect(shiftedRect, -delta_);
    msaView_->setSelection(msaRect_);

    if (!msaView_->isPartlyVisible(msaRect_))
        msaView_->centerOn(msaRect_);
}
