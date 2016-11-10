/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DSTRING_H
#define DSTRING_H

#include "AbstractAnonSeq.h"
#include "../enums.h"

struct DstringPod;

class Dstring : public AbstractAnonSeq
{
public:
    typedef boost::shared_ptr<Dstring> SPtr;
    static const int kType;

    Dstring(int id, const Seq &seq);
    explicit Dstring(const DstringPod &dstringPod);

    bool operator==(const Dstring &other) const
    {
        return id() == other.id();
    }
    bool operator!=(const Dstring &other) const
    {
        return !operator==(other);
    }

    int type() const
    {
        return eDstringEntity;
    }

    static Dstring *createEntity(const Seq &seq);
};

Q_DECLARE_TYPEINFO(Dstring, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<Dstring> DstringSPtr;

struct DstringPod : public AbstractAnonSeqPod
{
    DstringPod(int id = 0) : AbstractAnonSeqPod(id)
    {
    }
};

Q_DECLARE_TYPEINFO(DstringPod, Q_MOVABLE_TYPE);

#endif // DSTRING_H
