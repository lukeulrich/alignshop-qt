/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DBDNAMSACRUD_H
#define DBDNAMSACRUD_H

#include "AbstractDbEntityCrud.h"
#include "IMsaCrud.h"

class DnaMsa;
struct DnaMsaPod;
struct MsaMembersPod;

class DbDnaMsaCrud : public AbstractDbEntityCrud<DnaMsa, DnaMsaPod>,
                     public IMsaCrud<DnaMsa, DnaMsaPod>
{
public:
    DbDnaMsaCrud(IDbSource *dbSource);

    virtual void erase(const QVector<DnaMsa *> &aminoMsas);
    virtual void eraseByIds(const QVector<int> &ids);
    virtual QVector<DnaMsaPod> read(const QVector<int> &ids);
    virtual void save(const QVector<DnaMsa *> &aminoMsas);

    int countMembers(int aminoMsaId);
    virtual MsaMembersPod readMsaMembers(int msaId, int offset = 0, int limit = -1);

private:
    void insert(DnaMsa *aminoMsa) const;
    void insertCoreDnaMsa(DnaMsa *aminoMsa) const;
    void insertDnaMsaMembers(const DnaMsa *aminoMsa) const;
    void update(const DnaMsa *aminoMsa) const;
    void updateCoreDnaMsa(const DnaMsa *aminoMsa) const;
    void deleteOldDnaMsaMembers(const DnaMsa *aminoMsa) const;
};

#endif // DBDNAMSACRUD_H
