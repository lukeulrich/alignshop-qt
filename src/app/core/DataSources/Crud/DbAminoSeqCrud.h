/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DBAMINOSEQCRUD_H
#define DBAMINOSEQCRUD_H

#include "AbstractDbEntityCrud.h"

class AminoSeq;
struct AminoSeqPod;

class DbAminoSeqCrud : public AbstractDbEntityCrud<AminoSeq, AminoSeqPod>
{
public:
    DbAminoSeqCrud(IDbSource *dbSource);

    virtual void save(const QVector<AminoSeq *> &aminoSeqs);
    virtual void erase(const QVector<AminoSeq *> &aminoSeqs);
    virtual void eraseByIds(const QVector<int> &ids);
//    virtual QVector<AminoSeq *> read(const QVector<int> &ids);
    virtual QVector<AminoSeqPod> read(const QVector<int> &ids);

private:
    void insert(AminoSeq *aminoSeq);
    void update(const AminoSeq *aminoSeq);
};

#endif // DBAMINOSEQCRUD_H
