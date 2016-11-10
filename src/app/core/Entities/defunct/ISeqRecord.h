/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ISEQRECORD_H
#define ISEQRECORD_H

#include "IAnonSeq.h"
#include "IEntity.h"
#include "../Seq.h"

class ISeqRecord : public IEntity
{
public:
    ISeqRecord(int id, const QString &name, const QString &description)
        : IEntity(id), name_(name), description_(description)
    {
    }

    Seq seq() const
    {
        ASSERT(anonSeq());
        return anonSeq()->seq_;
    }

    IAnonSeq *anonSeq() const = 0;

    QString name_;
    QString description_;
};

#endif // ISEQRECORD_H
