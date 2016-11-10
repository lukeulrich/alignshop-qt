/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef REMOVEROWSCOMMAND_H
#define REMOVEROWSCOMMAND_H

#include <QtCore/QVector>
#include <QtGui/QUndoCommand>
#include "../../../core/util/ClosedIntRange.h"
#include "../../../core/global.h"
#include "../../../core/Entities/AbstractMsa.h"

class Subseq;

class RemoveRowsCommand : public QUndoCommand
{
public:
    RemoveRowsCommand(const AbstractMsaSPtr &msaEntity, const ClosedIntRange &rows, QUndoCommand *parentCommand = nullptr);
    ~RemoveRowsCommand();

    void redo();
    void undo();

private:
    AbstractMsaSPtr msaEntity_;
    ClosedIntRange rows_;
    QVector<Subseq *> removedSubseqs_;
};

#endif // REMOVEROWSCOMMAND_H
