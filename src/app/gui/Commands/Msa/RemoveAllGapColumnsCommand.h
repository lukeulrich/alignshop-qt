/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef REMOVEALLGAPCOLUMNSCOMMAND_H
#define REMOVEALLGAPCOLUMNSCOMMAND_H

#include <QtCore/QVector>

#include "../AbstractSkipFirstRedoCommand.h"
#include "../../../core/util/ClosedIntRange.h"
#include "../../../core/global.h"

class ObservableMsa;

class RemoveAllGapColumnsCommand : public AbstractSkipFirstRedoCommand
{
public:
    RemoveAllGapColumnsCommand(ObservableMsa *msa,
                               const QVector<ClosedIntRange> &removedColumns,
                               QUndoCommand *parentCommand = nullptr);

    void redoDelegate();
    void undo();

private:
    ObservableMsa *msa_;
    QVector<ClosedIntRange> removedColumns_;
};

#endif // REMOVEALLGAPCOLUMNSCOMMAND_H
