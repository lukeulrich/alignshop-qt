/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTSKIPFIRSTREDOCOMMAND_H
#define ABSTRACTSKIPFIRSTREDOCOMMAND_H

#include <QtGui/QUndoCommand>

/**
  * AbstractSkipFirstRedoCommand encapsulates those commands that when pushed onto the UndoStack should not have their
  * action performed.
  *
  * This approach is achieved by implementing the redo method and calling redoDelegate() for all but the first time
  * that redo() is called.
  */
class AbstractSkipFirstRedoCommand : public QUndoCommand
{
public:
    virtual void redo()
    {
        if (!firstTime_)
            redoDelegate();
        firstTime_ = false;
    }

    virtual void redoDelegate() = 0;

protected:
    AbstractSkipFirstRedoCommand(QUndoCommand *parent) : QUndoCommand(parent), firstTime_(true)   {}

private:
    bool firstTime_;
};

#endif // ABSTRACTSKIPFIRSTREDOCOMMAND_H
