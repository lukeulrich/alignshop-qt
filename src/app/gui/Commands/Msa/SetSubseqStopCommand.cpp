/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QScrollBar>

#include "SetSubseqStopCommand.h"
#include "../CommandIds.h"
#include "../../../core/ObservableMsa.h"
#include "../../../core/macros.h"
#include "../../widgets/AbstractMsaView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [ObservableMsa *]
  * @param row [int]
  * @param newStart [int]
  * @param msaView [AbstractMsaView *]
  * @param parentCommand [QUndoCommand *]
  */
SetSubseqStopCommand::SetSubseqStopCommand(ObservableMsa *msa,
                                           int row,
                                           int newStop,
                                           AbstractMsaView *msaView,
                                           QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      msa_(msa),
      row_(row),
      newStop_(newStop),
      msaView_(msaView),
      gapColumnsInserted_(0)
{
    ASSERT(msa_ != nullptr);
    setText(QString("(Row %1) Set stop to %2").arg(row).arg(newStop));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int SetSubseqStopCommand::id() const
{
    return Ag::eSetSubseqStopCommandId;
}

/**
  * Currently only support merging those commands that have the same operation.
  *
  * @param other [const QUndoCommand *]
  * @returns bool
  */
bool SetSubseqStopCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const SetSubseqStopCommand *otherCommand = static_cast<const SetSubseqStopCommand *>(other);
    bool isCompatibleMerge = msa_ == otherCommand->msa_ &&
                             row_ == otherCommand->row_ &&
                             changePod_.operation_ == otherCommand->changePod_.operation_;
    if (!isCompatibleMerge)
        return false;

    setText(QString("(Row %1) Set stop to %2").arg(row_).arg(otherCommand->newStop_));
    newStop_ = otherCommand->newStop_;
    if (changePod_.operation_ == SubseqChangePod::eExtendRight)
    {
        gapColumnsInserted_ += otherCommand->gapColumnsInserted_;
        changePod_.difference_ += otherCommand->changePod_.difference_;
        changePod_.columns_.end_ = otherCommand->changePod_.columns_.end_;
    }
    else if (changePod_.operation_ == SubseqChangePod::eTrimRight)
    {
        changePod_.difference_.prepend(otherCommand->changePod_.difference_);
        changePod_.columns_.begin_ = otherCommand->changePod_.columns_.begin_;
    }
    else
    {
        ASSERT(0);
        return false;
    }

    return true;
}

/**
  */
void SetSubseqStopCommand::redo()
{
    ASSERT_X(row_ >= 0 && row_ <= msa_->rowCount(), "Row out of range");
    ASSERT_X(msa_->at(row_)->stop() != newStop_, "New start position must be different than current start");

    bool scrollToMax = msaView_ != nullptr &&
                       msaView_->horizontalScrollBar()->value() == msaView_->horizontalScrollBar()->maximum();

    if (newStop_ > msa_->at(row_)->stop())
    {
        // Is it necessary to insert any gap columns?
        int nNewChars = newStop_ - msa_->at(row_)->stop();
        gapColumnsInserted_ = qMax(0, nNewChars - msa_->at(row_)->tailGaps());
        msa_->insertGapColumns(msa_->length() + 1, gapColumnsInserted_);
    }

    // Now update the subseq start position
    changePod_ = msa_->setSubseqStop(row_, newStop_);

    if (scrollToMax)
        msaView_->horizontalScrollBar()->setValue(msaView_->horizontalScrollBar()->maximum());
}

/**
  */
void SetSubseqStopCommand::undo()
{
    msa_->undo(SubseqChangePodVector() << changePod_);

    if (gapColumnsInserted_ > 0)
        msa_->removeGapColumns(ClosedIntRange(msa_->length() - gapColumnsInserted_ + 1, msa_->length()));
}
