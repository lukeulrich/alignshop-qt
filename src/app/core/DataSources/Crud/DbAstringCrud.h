/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DBASTRINGCRUD_H
#define DBASTRINGCRUD_H

#include "AbstractDbEntityCrud.h"
#include "IAnonSeqEntityCrud.h"

class Astring;
struct AstringPod;
class Coil;
class IDbSource;
struct Q3Prediction;
class Seg;

class DbAstringCrud : public AbstractDbEntityCrud<Astring, AstringPod>,
                      public IAnonSeqEntityCrud<Astring, AstringPod>
{
public:
    DbAstringCrud(IDbSource *dbSource);

    virtual void erase(const QVector<Astring *> &astrings);
    virtual void eraseByIds(const QVector<int> &ids);
//    virtual QVector<Astring *> read(const QVector<int> &ids);
//    virtual QVector<Astring *> readByDigests(const QVector<QByteArray> &digests);
    virtual QVector<AstringPod> read(const QVector<int> &ids);
    virtual QVector<AstringPod> readByDigests(const QVector<QByteArray> &digests);
    virtual void save(const QVector<Astring *> &astrings);

private:
    void insertCoreAstring(Astring *astring) const;
    QVector<Coil> readCoils(int astringId, int maxStop) const;
    QVector<Seg> readSegs(int astringId, int maxStop) const;
    Q3Prediction readQ3(int astringId) const;
    void saveCoils(int astringId, QVector<Coil> &coils) const;
    void saveSegs(int astringId, QVector<Seg> &segs) const;
    void saveQ3(int astringId, const Q3Prediction &q3) const;
};

#endif // DBASTRINGCRUD_H
