/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DBDSTRINGCRUD_H
#define DBDSTRINGCRUD_H

#include "AbstractDbEntityCrud.h"
#include "IAnonSeqEntityCrud.h"

class Dstring;
struct DstringPod;

class DbDstringCrud : public AbstractDbEntityCrud<Dstring, DstringPod>,
                      public IAnonSeqEntityCrud<Dstring, DstringPod>
{
public:
    DbDstringCrud(IDbSource *dbSource);

    virtual void erase(const QVector<Dstring *> &dstrings);
    virtual void eraseByIds(const QVector<int> &ids);
//    virtual QVector<Dstring *> read(const QVector<int> &ids);
//    virtual QVector<Dstring *> readByDigests(const QVector<QByteArray> &digests);
    virtual QVector<DstringPod> read(const QVector<int> &ids);
    virtual QVector<DstringPod> readByDigests(const QVector<QByteArray> &digests);
    virtual void save(const QVector<Dstring *> &dstrings);
};

#endif // DBDSTRINGCRUD_H
