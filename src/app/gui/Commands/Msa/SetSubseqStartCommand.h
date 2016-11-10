/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SETSUBSEQSTARTCOMMAND_H
#define SETSUBSEQSTARTCOMMAND_H

#include <QtGui/QUndoCommand>

#include "../../../core/PODs/SubseqChangePod.h"
#include "../../../core/global.h"

class ObservableMsa;

class SetSubseqStartCommand : public QUndoCommand
{
public:
    SetSubseqStartCommand(ObservableMsa *msa, int row, int newStart, QUndoCommand *parentCommand = nullptr);

    int id() const;
    bool mergeWith(const QUndoCommand *other);
    void redo();
    void undo();

private:
    ObservableMsa *msa_;
    int row_;
    int newStart_;
    int gapColumnsInserted_;
    SubseqChangePod changePod_;
};

#endif // SETSUBSEQSTARTCOMMAND_H
