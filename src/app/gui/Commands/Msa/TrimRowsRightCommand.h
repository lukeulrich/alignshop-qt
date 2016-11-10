/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TRIMROWSRIGHTCOMMAND_H
#define TRIMROWSRIGHTCOMMAND_H

#include <QtCore/QVector>
#include <QtGui/QUndoCommand>

#include "../../../core/PODs/SubseqChangePod.h"
#include "../../../core/util/ClosedIntRange.h"
#include "../../../core/global.h"

class ObservableMsa;

class TrimRowsRightCommand : public QUndoCommand
{
public:
    TrimRowsRightCommand(ObservableMsa *msa,
                        int msaColumn,
                        const ClosedIntRange &rows,
                        QUndoCommand *parentCommand = nullptr);

    void redo();
    void undo();

private:
    ObservableMsa *msa_;
    int msaColumn_;
    ClosedIntRange rows_;
    SubseqChangePodVector changePodVector_;     // To remember the changes that we have done
};

#endif // TRIMROWSRIGHTCOMMAND_H
