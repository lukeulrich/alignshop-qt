/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SetDnaSeqPrimerNameCommand.h"
#include "IPrimerMutator.h"
#include "Primer.h"
#include "../core/Entities/EntityFlags.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
SetDnaSeqPrimerNameCommand::SetDnaSeqPrimerNameCommand(const DnaSeqSPtr &dnaSeq, const int primerIndex, const QString &newName, IPrimerMutator *primerMutator, QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      dnaSeq_(dnaSeq),
      primerIndex_(primerIndex),
      newName_(newName),
      primerMutator_(primerMutator)
{
    saveOldName();
    setDescriptiveText();
    primersDirtyBeforeCommand_ = dnaSeq->isDirty(Ag::ePrimersFlag);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void SetDnaSeqPrimerNameCommand::redo()
{
    setPrimerName(newName_);
    dnaSeq_->setDirty(Ag::ePrimersFlag, true);
}

void SetDnaSeqPrimerNameCommand::undo()
{
    setPrimerName(oldName_);
    if (!primersDirtyBeforeCommand_)
        dnaSeq_->setDirty(Ag::ePrimersFlag, false);
}


// ------------------------------------------------------------------------------------------------
// Private methods
void SetDnaSeqPrimerNameCommand::saveOldName()
{
    oldName_ = primer().name();
}

void SetDnaSeqPrimerNameCommand::setDescriptiveText()
{
    QString friendlyOldName = oldName_;
    if (friendlyOldName.isEmpty())
        friendlyOldName = "[No name]";

    setText(QString("Rename primer %1 -> %2").arg(friendlyOldName, newName_));
}

void SetDnaSeqPrimerNameCommand::setPrimerName(const QString &name)
{
    primerMutator_->setPrimerName(primer(), name);
}

Primer&SetDnaSeqPrimerNameCommand::primer()
{
    return dnaSeq_->primers_[primerIndex_];
}
