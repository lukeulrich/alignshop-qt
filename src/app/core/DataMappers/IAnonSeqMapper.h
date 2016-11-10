/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IANONSEQMAPPER_H
#define IANONSEQMAPPER_H

#include "IEntityMapper.h"

template<typename T>
class IAnonSeqMapper : public virtual IEntityMapper<T>
{
public:
    virtual T *findOneByDigest(const QByteArray &digest) const = 0;
    virtual QVector<T *> findByDigests(const QVector<QByteArray> &digests) const = 0;
};

#endif // IANONSEQMAPPER_H
