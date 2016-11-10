/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef REMOVEDNASEQPRIMERSCOMMAND_H
#define REMOVEDNASEQPRIMERSCOMMAND_H

#include <QtGui/QUndoCommand>
#include "../core/Entities/DnaSeq.h"
#include "../core/global.h"

class DnaSeqPrimerVectorMutator;

class RemoveDnaSeqPrimersCommand : public QUndoCommand
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    RemoveDnaSeqPrimersCommand(const DnaSeqSPtr &dnaSeq, const ClosedIntRange &removeRange, DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator, QUndoCommand *parentCommand = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void redo();
    void undo();


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    DnaSeqSPtr dnaSeq_;
    ClosedIntRange removeRange_;
    DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator_;
    PrimerVector removedPrimers_;
};

#endif // REMOVEDNASEQPRIMERSCOMMAND_H
