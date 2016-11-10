/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNASEQCOMMANDPRIMERMUTATOR_H
#define DNASEQCOMMANDPRIMERMUTATOR_H

#include "IPrimerMutator.h"
#include "../core/Entities/DnaSeq.h"

class QUndoStack;

/**
  * DnaSeqCommandPrimerMutator proxies all mutator interface methods into corresponding UndoCommands that utilize a
  * real PrimerPairMutator for effecting the actual change.
  *
  * If no valid undo
  */
class DnaSeqCommandPrimerMutator : public IPrimerMutator
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    DnaSeqCommandPrimerMutator(IPrimerMutator *sourcePrimerMutator, QUndoStack *undoStack, QObject *parent = 0);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setUndoStack(QUndoStack *undoStack);
    virtual bool setPrimerName(Primer &primer, const QString &newName);


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setDnaSeq(const DnaSeqSPtr &dnaSeq);


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    DnaSeqSPtr dnaSeq_;
    IPrimerMutator *sourcePrimerMutator_;
    QUndoStack *undoStack_;
};

#endif // DNASEQCOMMANDPRIMERMUTATOR_H
