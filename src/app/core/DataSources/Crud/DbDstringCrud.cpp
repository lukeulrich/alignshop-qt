/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVariant>
#include <QtCore/QScopedPointer>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include "DbDstringCrud.h"
#include "../../Entities/Dstring.h"
#include "../../Entities/EntityFlags.h"
#include "../../enums.h"
#include "../../global.h"
#include "../../macros.h"
#include "../IDbSource.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param dbSource [IDbSource *]
  */
DbDstringCrud::DbDstringCrud(IDbSource *dbSource) : AbstractDbEntityCrud<Dstring, DstringPod>(dbSource)
{
    ASSERT(dbSource != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param dstrings [const QVector<Dstring *> &]
  */
void DbDstringCrud::erase(const QVector<Dstring *> & /* dstrings */)
{
    ASSERT_X(0, "Not allowed to erase Dstring via the CRUD!");
}

/**
  * @param ids [const QVector<int> &]
  */
void DbDstringCrud::eraseByIds(const QVector<int> & /* ids */)
{
    ASSERT_X(0, "Not allowed to erase Dstring via the CRUD!");
}

/**
  * @param ids [const QVector<int> &]
  * @returns QVector<DstringPod>
  */
QVector<DstringPod> DbDstringCrud::read(const QVector<int> &ids)
{
    // Gather the base information
    QSqlQuery query = dbSource()->getPreparedQuery("readDstring",
                                                   "SELECT id, sequence "
                                                   "FROM dstrings "
                                                   "WHERE id = ?");

    QVector<DstringPod> dstringPods;
    dstringPods.reserve(ids.size());
    foreach (int id, ids)
    {
        query.bindValue(0, id);
        if (!query.exec())
            throw 0;

        if (!query.next())
        {
            dstringPods << DstringPod();
            continue;
        }

        dstringPods << DstringPod(query.value(0).toInt());
        dstringPods.last().seq_ = Seq(query.value(1).toByteArray(), eDnaGrammar);
    }

    query.finish();

    return dstringPods;
}

/**
  * @param digests [const QVector<QByteArray> &]
  * @returns QVector<DstringPod>
  */
QVector<DstringPod> DbDstringCrud::readByDigests(const QVector<QByteArray> &digests)
{
    // Gather the base information
    QSqlQuery query = dbSource()->getPreparedQuery("readDstringViaDigest",
                                                   "SELECT id, sequence "
                                                   "FROM dstrings "
                                                   "WHERE digest = ?");

    QVector<DstringPod> dstringPods;
    dstringPods.reserve(digests.size());
    foreach (const QByteArray &digest, digests)
    {
        query.bindValue(0, digest);
        if (!query.exec())
            throw 0;

        if (!query.next())
        {
            dstringPods << DstringPod();
            continue;
        }

        dstringPods << DstringPod(query.value(0).toInt());
        dstringPods.last().seq_ = Seq(query.value(1).toByteArray(), eDnaGrammar);
    }

    query.finish();

    return dstringPods;
}

/**
  * @param dstrings [const QVector<Dstring *> &]
  */
void DbDstringCrud::save(const QVector<Dstring *> &dstrings)
{
    QSqlQuery insert = dbSource()->getPreparedQuery("insertDstring",
                                                    "INSERT INTO dstrings (digest, length, sequence) "
                                                    "VALUES (?, ?, ?)");
    foreach (Dstring *dstring, dstrings)
    {
        ASSERT(dstring);
        if (dstring->isNew())
        {
            insert.bindValue(0, dstring->seq_.digest());
            insert.bindValue(1, dstring->seq_.length());
            insert.bindValue(2, dstring->seq_.asByteArray());
            if (!insert.exec())
                throw 0;

            dstring->setId(insert.lastInsertId().toInt());

            dstring->setDirty(Ag::eCoreDataFlag, false);
        }
    }

    insert.finish();
}
