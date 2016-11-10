/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONDITIONALUNDOCOMMAND_H
#define CONDITIONALUNDOCOMMAND_H

#include <QtGui/QUndoCommand>
#include "../../core/global.h"

/**
  * ConditionalUndoCommand provides a control point for conditionally performing an undo operation.
  *
  * Motivation: Commands may involve external state and it is frequently desirable to provide a means for confirming
  *             that an undo should continue relevant to this external state. ConditionalUndoCommand provides such a
  *             mechanism via the virtual method acceptUndo().
  *
  * An instance where a conditional undo is desirable occurs when associating an AdocTreeNode with a specific task. For
  * example, the BLASTing of a sequence. When the BLAST command is queued, a corresponding AdocTreeNode is created in
  * the data tree. If the user attempts to undo, ConditionalUndoCommand provides a control point for confirming this
  * action before removing the node from the data tree. In contrast, the standard QUndoCommand would automatically
  * undo without any confirmation. This latter mode works well for small, atomic operations; however, when dealing with
  * long-running operations such as batch tasks, it is not as suitable.
  *
  * To use the facility provided by this class, it is necessary to perform the relevant test before calling undo on the
  * undostack. This is done by first checking if the last command on the QUndoStack is a ConditionalUndoCommand. If this
  * is not the case then simply perform the undo. Otherwise, perform the undo only if acceptUndo returns true.
  *
  * One such implementation might be:
  *
  * QUndoStack *stack;
  * Q_SLOT void onUndoRequested()
  * {
  *     if (!stack->canUndo())
  *         return;
  *     const QUndoCommand *command = stack->command(stack->index());
  *     const ConditionalUndoCommand *conditionalCommand = dynamic_cast<const ConditionalUndoCommand *>(command);
  *     if (conditionalCommand && !conditionalCommand->acceptUndo())
  *         return;
  *     stack->undo();
  * }
  */
class ConditionalUndoCommand : public QUndoCommand
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructr
    ConditionalUndoCommand(QUndoCommand *parentCommand = nullptr)
        : QUndoCommand(parentCommand)
    {
    }

    ConditionalUndoCommand(const QString &text, QUndoCommand *parentCommand = nullptr)
        : QUndoCommand(text, parentCommand)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual bool acceptUndo() const                 //!< Returns true if an undo operation should proceed; false if this or any conditional undo child returns false. Default implementation returns true
    {
        for (int i=0, z=childCount(); i<z; ++i)
        {
            const ConditionalUndoCommand *childConditionalUndoCommand = dynamic_cast<const ConditionalUndoCommand *>(child(i));
            if (childConditionalUndoCommand &&
                !childConditionalUndoCommand->acceptUndo())
            {
                return false;
            }
        }

        return true;
    }

    QString reason() const
    {
        return reason_;
    }

    void setReason(const QString &reason)
    {
        reason_ = reason;
    }

private:
    QString reason_;
};

#endif // CONDITIONALUNDOCOMMAND_H
