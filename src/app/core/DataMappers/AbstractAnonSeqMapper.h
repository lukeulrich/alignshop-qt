/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTANONSEQMAPPER_H
#define ABSTRACTANONSEQMAPPER_H

#include "IAnonSeqMapper.h"
#include "../global.h"

template<typename T>
class AbstractAnonSeqMapper : public IAnonSeqMapper<T>
{
public:
    virtual T *findOneByDigest(const QByteArray &digest) const
    {
        static QVector<QByteArray> digests(1, nullptr);
        digests[0] = digest;
        return this->findByDigests(digests).first();
    }
};

#endif // ABSTRACTANONSEQMAPPER_H
