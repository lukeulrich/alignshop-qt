/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SetSubseqStartCommand.h"
#include "../CommandIds.h"
#include "../../../core/ObservableMsa.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [ObservableMsa *]
  * @param row [int]
  * @param newStart [int]
  * @param parentCommand [QUndoCommand *]
  */
SetSubseqStartCommand::SetSubseqStartCommand(ObservableMsa *msa, int row, int newStart, QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand), msa_(msa), row_(row), newStart_(newStart), gapColumnsInserted_(0)
{
    ASSERT(msa_ != nullptr);
    setText(QString("(Row %1) Set start to %2").arg(row).arg(newStart));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int SetSubseqStartCommand::id() const
{
    return Ag::eSetSubseqStartCommandId;
}

/**
  * Currently only support merging those commands that have the same operation.
  *
  * @param other [const QUndoCommand *]
  * @returns bool
  */
bool SetSubseqStartCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const SetSubseqStartCommand *otherCommand = static_cast<const SetSubseqStartCommand *>(other);
    bool isCompatibleMerge = msa_ == otherCommand->msa_ &&
                             row_ == otherCommand->row_ &&
                             changePod_.operation_ == otherCommand->changePod_.operation_;
    if (!isCompatibleMerge)
        return false;

    setText(QString("(Row %1) Set start to %2").arg(row_).arg(otherCommand->newStart_));
    newStart_ = otherCommand->newStart_;
    if (changePod_.operation_ == SubseqChangePod::eExtendLeft)
    {
        gapColumnsInserted_ += otherCommand->gapColumnsInserted_;
        changePod_.difference_.prepend(otherCommand->changePod_.difference_);
        changePod_.columns_.begin_ = otherCommand->changePod_.columns_.begin_;

        // The end column still refers to the column before any gap columns were inserted and thus will be out of
        // register in these cases. Update it to refer to the proper end column taking into account any gap columns.
        changePod_.columns_.end_ += otherCommand->gapColumnsInserted_;
    }
    else if (changePod_.operation_ == SubseqChangePod::eTrimLeft)
    {
        changePod_.difference_ += otherCommand->changePod_.difference_;
        changePod_.columns_.end_ = otherCommand->changePod_.columns_.end_;
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
void SetSubseqStartCommand::redo()
{
    ASSERT_X(row_ >= 0 && row_ <= msa_->rowCount(), "Row out of range");
    ASSERT_X(msa_->at(row_)->start() != newStart_, "New start position must be different than current start");

    if (newStart_ < msa_->at(row_)->start())
    {
        // Is it necessary to insert any gap columns?
        int nNewChars = msa_->at(row_)->start() - newStart_;
        gapColumnsInserted_ = qMax(0, nNewChars - msa_->at(row_)->headGaps());
        msa_->insertGapColumns(1, gapColumnsInserted_);
    }

    // Now update the subseq start position
    changePod_ = msa_->setSubseqStart(row_, newStart_);
}

/**
  */
void SetSubseqStartCommand::undo()
{
    msa_->undo(SubseqChangePodVector() << changePod_);

    if (gapColumnsInserted_ > 0)
        msa_->removeGapColumns(ClosedIntRange(1, gapColumnsInserted_));
}
