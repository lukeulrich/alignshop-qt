/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SETDNASEQPRIMERNAMECOMMAND_H
#define SETDNASEQPRIMERNAMECOMMAND_H

#include <QtGui/QUndoCommand>

#include "../core/Entities/DnaSeq.h"
#include "../core/global.h"

class Primer;
class IPrimerMutator;

class SetDnaSeqPrimerNameCommand : public QUndoCommand
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    SetDnaSeqPrimerNameCommand(const DnaSeqSPtr &dnaSeq, const int primerIndex, const QString &newName, IPrimerMutator *primerMutator, QUndoCommand *parentCommand = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual void redo();
    virtual void undo();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void saveOldName();
    void setDescriptiveText();
    void setPrimerName(const QString &name);
    Primer &primer();


    // ------------------------------------------------------------------------------------------------
    // Private members
    DnaSeqSPtr dnaSeq_;
    int primerIndex_;
    QString newName_;
    QString oldName_;
    IPrimerMutator *primerMutator_;
    bool primersDirtyBeforeCommand_;
};

#endif // SETDNASEQPRIMERAMECOMMAND_H
