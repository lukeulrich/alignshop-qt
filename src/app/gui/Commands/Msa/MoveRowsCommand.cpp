/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QStringList>

#include "MoveRowsCommand.h"
#include "../../../core/Entities/AbstractMsa.h"
#include "../../../core/ObservableMsa.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
MoveRowsCommand::MoveRowsCommand(ObservableMsa *msa, const ClosedIntRange &sourceRange, int destRow, QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      range_(sourceRange),
      destRow_(destRow),
      msa_(msa)
{
    ASSERT(msa_ != nullptr);
    ASSERT(isValidDestinationRow());

    // Construct a user-friendly undo text string
    QString moveText = "Move row";
    if (range_.begin_ != range_.end_)
        moveText += QString("s %1 .. %2 to row %3").arg(range_.begin_).arg(range_.end_).arg(destRow_);
    else
        moveText += QString(" %1 to row %2").arg(range_.begin_).arg(destRow_);

//    QStringList moveRanges;
//    foreach (const ClosedIntRange &sourceRange, sourceRanges)
//    {
//        if (sourceRange.begin_ != sourceRange.end_)
//            moveRanges << QString("%1 .. %2").arg(sourceRange.begin_).arg(sourceRange.end_);
//        else
//            moveRanges << QString::number(sourceRange.begin_);
//    }
//    moveText += moveRanges.join(", ") + " to row " + QString::number(destRow);
    setText(moveText);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void MoveRowsCommand::redo()
{
    msa_->moveRowRange(range_, destRow_);
    invertOperation();
}

void MoveRowsCommand::undo()
{
    redo();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
bool MoveRowsCommand::isValidDestinationRow() const
{
    if (!msa_->isValidRow(destRow_))
        return false;

    if (!msa_->isValidRowRange(range_))
        return false;

//    foreach (const ClosedIntRange &range, sourceRange_)
//        if (range.length() > 1 && destRow_ > range.begin_ && destRow_ <= range.end_)
//            return false;

    return true;
}

void MoveRowsCommand::invertOperation()
{
    int nRows = range_.length();
    qSwap(range_.begin_, destRow_);
    range_.end_ = range_.begin_ + nRows - 1;
}
