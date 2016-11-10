/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DnaSeqCommandPrimerMutator.h"
#include "SetDnaSeqPrimerNameCommand.h"
// #include "Primer.h"

#include "../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
DnaSeqCommandPrimerMutator::DnaSeqCommandPrimerMutator(IPrimerMutator *sourcePrimerMutator, QUndoStack *undoStack, QObject *parent)
    : IPrimerMutator(parent),
      sourcePrimerMutator_(sourcePrimerMutator),
      undoStack_(undoStack)
{
    ASSERT(dynamic_cast<DnaSeqCommandPrimerMutator *>(sourcePrimerMutator) == 0);
    ASSERT(sourcePrimerMutator != nullptr);
    ASSERT(undoStack != nullptr);

    // Since we are behaving as a proxy, must forward all source signals
    connect(sourcePrimerMutator_, SIGNAL(primerNameChanged(int)), SIGNAL(primerNameChanged(int)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void DnaSeqCommandPrimerMutator::setUndoStack(QUndoStack *undoStack)
{
    ASSERT(undoStack != nullptr);
    undoStack_ = undoStack;
}

bool DnaSeqCommandPrimerMutator::setPrimerName(Primer &primer, const QString &newName)
{
    if (primer.name() == newName)
        return false;

    if (!dnaSeq_ || !undoStack_)
        return false;

    int dnaSeqPrimerIndex = Primer::primerRowById(dnaSeq_->primers_, primer.id());
    if (dnaSeqPrimerIndex == -1)
        return false;

    SetDnaSeqPrimerNameCommand *newNameCommand = new SetDnaSeqPrimerNameCommand(dnaSeq_,
                                                                                dnaSeqPrimerIndex,
                                                                                newName,
                                                                                sourcePrimerMutator_);
    undoStack_->push(newNameCommand);
    return true;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
void DnaSeqCommandPrimerMutator::setDnaSeq(const DnaSeqSPtr &dnaSeq)
{
    dnaSeq_ = dnaSeq;
}
