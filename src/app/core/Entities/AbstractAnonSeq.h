/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTANONSEQ_H
#define ABSTRACTANONSEQ_H

#include "AbstractEntity.h"
#include "../Seq.h"

class AbstractAnonSeq : public AbstractEntity
{
public:
    typedef boost::shared_ptr<AbstractAnonSeq> SPtr;

    const Seq seq_;

protected:
    AbstractAnonSeq(int id, const Seq &seq) : AbstractEntity(id), seq_(seq)
    {
    }
};

struct AbstractAnonSeqPod : public AbstractEntityPod
{
    Seq seq_;

    AbstractAnonSeqPod(int id) : AbstractEntityPod(id)
    {
    }
};

Q_DECLARE_TYPEINFO(AbstractAnonSeqPod, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<AbstractAnonSeq> AbstractAnonSeqSPtr;

#endif // ABSTRACTANONSEQ_H
