/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INSERTGAPCOLUMNSCOMMAND_H
#define INSERTGAPCOLUMNSCOMMAND_H

#include "../AbstractSkipFirstRedoCommand.h"
#include "../../../core/global.h"

class ObservableMsa;

class InsertGapColumnsCommand : public AbstractSkipFirstRedoCommand
{
public:
    InsertGapColumnsCommand(ObservableMsa *msa, int column, int count, QUndoCommand *parentCommand = nullptr);

    void redoDelegate();
    void undo();

private:
    ObservableMsa *msa_;
    int column_;
    int count_;
};

#endif // INSERTGAPCOLUMNSCOMMAND_H
