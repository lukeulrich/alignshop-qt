/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTMSACOMMAND_H
#define ABSTRACTMSACOMMAND_H

#include <QtGui/QUndoCommand>

#include "../../../core/PODs/SubseqChangePod.h"

// Make this class only work for extend, trim, and level operations as identified by a common id
class AbstractMsaCommand : public QUndoCommand
{
public:
    AbstractMsaCommand(ObservableMsa *msa);

    virtual bool mergeWith(const QUndoCommand *other)
    {
        if (other->id() != id())
            return false;

        AbstractMsaCommand *otherMsaCommand = static_cast<AbstractMsaCommand *>(other);
        if (otherMsaCommand->changePodVector_.isEmpty())
            return true;

        return false;
    }

    int id() const
    {
        return 1;
    }

protected:
    SubseqChangePodVector changePodVector_;     // To remember the changes that we have done
};

#endif // ABSTRACTMSACOMMAND_H
