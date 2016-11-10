/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SORTMSACOMMAND_H
#define SORTMSACOMMAND_H

#include <QtCore/QVector>
#include <QtGui/QUndoCommand>

#include "../../../core/global.h"

class ObservableMsa;
class ISubseqLessThan;
class Subseq;

class SortMsaCommand : public QUndoCommand
{
public:
    SortMsaCommand(ObservableMsa *msa,
                   ISubseqLessThan *subseqLessThan,
                   Qt::SortOrder sortOrder,
                   QUndoCommand *parentCommand = nullptr);
    ~SortMsaCommand();

    void redo();
    void undo();

private:
    ObservableMsa *msa_;
    ISubseqLessThan *subseqLessThan_;
    Qt::SortOrder sortOrder_;

    QVector<Subseq *> oldSubseqVector_;
};

#endif // SORTMSACOMMAND_H
