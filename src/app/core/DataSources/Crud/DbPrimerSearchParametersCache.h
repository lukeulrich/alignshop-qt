/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DBPRIMERSEARCHPARAMETERSCACHE_H
#define DBPRIMERSEARCHPARAMETERSCACHE_H

#include <QtCore/QHash>
#include <QtCore/QSharedPointer>
#include <QtCore/QVector>

#include "../../../primer/PrimerSearchParameters.h"

class IDbSource;

class DbPrimerSearchParametersCache
{
public:
    DbPrimerSearchParametersCache(IDbSource *dbSource);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void clearCache();
    void cacheRecords(const QVector<int> &primerSearchParametersIds);
    bool isCached(const int primerSearchParametersId) const;
    QSharedPointer<PrimerSearchParameters> read(const int primerSearchParametersId);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    QSharedPointer<PrimerSearchParameters> readFromDatabase(const int primerSearchParametersId) const;
    void insertIntoCache(const QSharedPointer<PrimerSearchParameters> &primerSearchParameters);

    IDbSource *dbSource_;
    QHash<int, QSharedPointer<PrimerSearchParameters> > cachedRecords_;
};

#endif // DBPRIMERSEARCHPARAMETERSCACHE_H
