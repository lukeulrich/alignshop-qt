/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IANONSEQENTITYCRUD_H
#define IANONSEQENTITYCRUD_H

#include "IEntityCrud.h"

template<typename T, typename PodT>
class IAnonSeqEntityCrud : public virtual IEntityCrud<T, PodT>
{
public:
//    virtual QVector<T *> readByDigests(const QVector<QByteArray> &digests) = 0;
    virtual QVector<PodT> readByDigests(const QVector<QByteArray> &digests) = 0;
};

#endif // IANONSEQENTITYCRUD_H
