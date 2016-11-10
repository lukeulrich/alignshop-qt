/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SETSUBSEQSTOPCOMMAND_H
#define SETSUBSEQSTOPCOMMAND_H

#include <QtGui/QUndoCommand>

#include "../../../core/PODs/SubseqChangePod.h"
#include "../../../core/global.h"

class ObservableMsa;
class AbstractMsaView;

class SetSubseqStopCommand : public QUndoCommand
{
public:
    SetSubseqStopCommand(ObservableMsa *msa,
                         int row,
                         int newStop,
                         AbstractMsaView *msaView = nullptr,
                         QUndoCommand *parentCommand = nullptr);

    int id() const;
    bool mergeWith(const QUndoCommand *other);
    void redo();
    void undo();

private:
    ObservableMsa *msa_;
    int row_;
    int newStop_;
    AbstractMsaView *msaView_;
    int gapColumnsInserted_;
    SubseqChangePod changePod_;
};

#endif // SETSUBSEQSTOPCOMMAND_H
