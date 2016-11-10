/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DBAMINOMSACRUD_H
#define DBAMINOMSACRUD_H

#include "AbstractDbEntityCrud.h"
#include "IMsaCrud.h"

class AminoMsa;
struct AminoMsaPod;
struct MsaMembersPod;

class DbAminoMsaCrud : public AbstractDbEntityCrud<AminoMsa, AminoMsaPod>,
                       public IMsaCrud<AminoMsa, AminoMsaPod>
{
public:
    DbAminoMsaCrud(IDbSource *dbSource);

    virtual void erase(const QVector<AminoMsa *> &aminoMsas);
    virtual void eraseByIds(const QVector<int> &ids);
    virtual QVector<AminoMsaPod> read(const QVector<int> &ids);
    virtual void save(const QVector<AminoMsa *> &aminoMsas);

    int countMembers(int aminoMsaId);
    virtual MsaMembersPod readMsaMembers(int msaId, int offset = 0, int limit = -1);

private:
    void insert(AminoMsa *aminoMsa) const;
    void insertCoreAminoMsa(AminoMsa *aminoMsa) const;
    void insertAminoMsaMembers(const AminoMsa *aminoMsa) const;
    void update(const AminoMsa *aminoMsa) const;
    void updateCoreAminoMsa(const AminoMsa *aminoMsa) const;
    void deleteOldAminoMsaMembers(const AminoMsa *aminoMsa) const;
};

#endif // DBAMINOMSACRUD_H
