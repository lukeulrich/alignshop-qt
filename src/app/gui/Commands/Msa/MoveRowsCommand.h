/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MOVEROWSCOMMAND_H
#define MOVEROWSCOMMAND_H

#include <QtCore/QVector>
#include <QtGui/QUndoCommand>
#include "../../../core/util/ClosedIntRange.h"
#include "../../../core/global.h"

class ObservableMsa;

class MoveRowsCommand : public QUndoCommand
{
public:
    MoveRowsCommand(ObservableMsa *msa, const ClosedIntRange &sourceRange, int destRow, QUndoCommand *parentCommand = nullptr);

    void redo();
    void undo();

private:
    bool isValidDestinationRow() const;
    void invertOperation();

    ClosedIntRange range_;
    int destRow_;
    ObservableMsa *msa_;
};

#endif // MOVEROWSCOMMAND_H
