/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef APPENDPRIMERSTODNASEQCOMMAND_H
#define APPENDPRIMERSTODNASEQCOMMAND_H

#include <QtGui/QUndoCommand>

#include "Primer.h"
#include "../core/Entities/DnaSeq.h"
#include "../core/global.h"

class DnaSeqPrimerVectorMutator;

class AppendPrimersToDnaSeqCommand : public QUndoCommand
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    AppendPrimersToDnaSeqCommand(const DnaSeqSPtr &dnaSeq, const PrimerVector &primers, DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator, QUndoCommand *parentCommand = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void redo();
    void undo();


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    DnaSeqSPtr dnaSeq_;
    PrimerVector primers_;
    DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator_;
    ClosedIntRange appendRange_;
};

#endif // APPENDPRIMERSTODNASEQCOMMAND_H
