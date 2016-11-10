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

#include "DbAstringCrud.h"
#include "../../Entities/Astring.h"
#include "../../Entities/EntityFlags.h"
#include "../../PODs/Q3Prediction.h"
#include "../../ValueObjects/Coil.h"
#include "../../ValueObjects/Seg.h"
#include "../../global.h"
#include "../../macros.h"
#include "../IDbSource.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param dbSource [IDbSource *]
  */
DbAstringCrud::DbAstringCrud(IDbSource *dbSource) : AbstractDbEntityCrud<Astring, AstringPod>(dbSource)
{
    ASSERT(dbSource != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param astrings [const QVector<Astring *> &]
  */
void DbAstringCrud::erase(const QVector<Astring *> & /* astrings */)
{
    ASSERT_X(0, "Not allowed to erase Astring via the CRUD!");
}

void DbAstringCrud::eraseByIds(const QVector<int> & /* ids */)
{
    ASSERT_X(0, "Not allowed to erase Astring via the CRUD!");
}

/**
  * @param ids [const QVector<int> &]
  * @returns QVector<AstringPod>
  */
QVector<AstringPod> DbAstringCrud::read(const QVector<int> &ids)
{
    // Gather the base information
    QSqlQuery query = dbSource()->getPreparedQuery("readAstring",
                                                   "SELECT id, sequence "
                                                   "FROM astrings "
                                                   "WHERE id = ?");

    QVector<AstringPod> astringPods;
    astringPods.reserve(ids.size());
    foreach (int id, ids)
    {
        query.bindValue(0, id);
        if (!query.exec())
            throw 0;

        if (!query.next())
        {
            astringPods << AstringPod();
            continue;
        }

        astringPods << AstringPod(query.value(0).toInt());
        AstringPod &pod = astringPods.last();
        pod.seq_ = Seq(query.value(1).toByteArray(), eAminoGrammar);
        try
        {
            pod.coils_ = readCoils(pod.id_, pod.seq_.length());
            pod.segs_ = readSegs(pod.id_, pod.seq_.length());
            pod.q3_ = readQ3(pod.id_);
        }
        catch(...)
        {
            throw;
        }
    }

    query.finish();

    return astringPods;
}

/**
  * @param digests [const QVector<QByteArray> &]
  * @return QVector<AstringPod>
  */
QVector<AstringPod> DbAstringCrud::readByDigests(const QVector<QByteArray> &digests)
{
    // Gather the base information
    QSqlQuery query = dbSource()->getPreparedQuery("readAstringViaDigest",
                                                   "SELECT id, sequence "
                                                   "FROM astrings "
                                                   "WHERE digest = ?");

    QVector<AstringPod> astringPods;
    astringPods.reserve(digests.size());
    foreach (const QByteArray &digest, digests)
    {
        query.bindValue(0, digest);
        if (!query.exec())
            throw 0;

        if (!query.next())
        {
            astringPods << AstringPod();
            continue;
        }

        astringPods << AstringPod(query.value(0).toInt());
        AstringPod &pod = astringPods.last();
        pod.seq_ = Seq(query.value(1).toByteArray(), eAminoGrammar);
        try
        {
            pod.coils_ = readCoils(pod.id_, pod.seq_.length());
            pod.segs_ = readSegs(pod.id_, pod.seq_.length());
            pod.q3_ = readQ3(pod.id_);
        }
        catch(...)
        {
            throw;
        }
    }

    query.finish();

    return astringPods;
}

/**
  * @param astrings [const QVector<Astring *> &]
  */
void DbAstringCrud::save(const QVector<Astring *> &astrings)
{
    using namespace Ag;

    foreach (Astring *astring, astrings)
    {
        if (astring->isNew())
            insertCoreAstring(astring);

        // Coils
        if (astring->isDirty(eCoilsFlag))
        {
            QVector<Coil> coils = astring->coils();
            saveCoils(astring->id(), coils);
            astring->setCoils(coils);

            astring->setDirty(eCoilsFlag, false);
        }

        // Segs
        if (astring->isDirty(eSegsFlag))
        {
            QVector<Seg> segs = astring->segs();
            saveSegs(astring->id(), segs);
            astring->setSegs(segs);

            astring->setDirty(eSegsFlag, false);
        }

        // Q3 Prediction
        if (astring->isDirty(eQ3Flag))
        {
            saveQ3(astring->id(), astring->q3());
            astring->setDirty(eQ3Flag, false);
        }
    }
}



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param astring [Astring *]
  */
void DbAstringCrud::insertCoreAstring(Astring *astring) const
{
    ASSERT(astring);
    ASSERT(astring->isNew());

    QSqlQuery insert = dbSource()->getPreparedQuery("insertAstring",
                                                    "INSERT INTO astrings (digest, length, sequence) "
                                                    "VALUES (?, ?, ?)");

    insert.bindValue(0, astring->seq_.digest());
    insert.bindValue(1, astring->seq_.length());
    insert.bindValue(2, astring->seq_.asByteArray());
    if (!insert.exec())
        throw 0;

    astring->setId(insert.lastInsertId().toInt());

    insert.finish();

    astring->setDirty(Ag::eCoreDataFlag, false);
}

/**
  * @param astringId [int]
  * @param maxStop [int]
  * @returns QVector<Coil>
  */
QVector<Coil> DbAstringCrud::readCoils(int astringId, int maxStop) const
{
    ASSERT(maxStop > 0);
    QSqlQuery query = dbSource()->getPreparedQuery("readCoils",
                                                   "SELECT id, start, stop "
                                                   "FROM coils "
                                                   "WHERE astring_id = ? AND start > 0 AND stop >= start AND stop <= ? " // Ensure they are in the proper range
                                                   "GROUP BY start, stop "    // Prevent duplicates
                                                   "ORDER BY start");

    // --------------------------
    // --------------------------
    query.bindValue(0, astringId);
    query.bindValue(1, maxStop);
    if (!query.exec())
        throw 0;

    QVector<Coil> coils;
    while (query.next())
        coils << Coil(query.value(0).toInt(), ClosedIntRange(query.value(1).toInt(), query.value(2).toInt()));

    query.finish();

    return coils;
}

/**
  * @param astringId [int]
  * @param maxStop [int]
  * @returns QVector<Seg>
  */
QVector<Seg> DbAstringCrud::readSegs(int astringId, int maxStop) const
{
    ASSERT(maxStop > 0);
    QSqlQuery query = dbSource()->getPreparedQuery("readSegs",
                                                   "SELECT id, start, stop "
                                                   "FROM segs "
                                                   "WHERE astring_id = ? AND start > 0 AND stop >= start AND stop <= ? " // Ensure they are in the proper range
                                                   "GROUP BY start, stop "    // Prevent duplicates
                                                   "ORDER BY start");

    // --------------------------
    // --------------------------
    query.bindValue(0, astringId);
    query.bindValue(1, maxStop);
    if (!query.exec())
        throw 0;

    QVector<Seg> segs;
    while (query.next())
        segs << Seg(query.value(0).toInt(), ClosedIntRange(query.value(1).toInt(), query.value(2).toInt()));

    query.finish();

    return segs;
}

/**
  * @param astringId [int astringId]
  * @returns Q3Prediction
  */
Q3Prediction DbAstringCrud::readQ3(int astringId) const
{
    QSqlQuery query = dbSource()->getPreparedQuery("readQ3",
                                                   "SELECT q3, confidence "
                                                   "FROM q3 "
                                                   "WHERE astring_id = ?");

    // --------------------------
    // --------------------------
    query.bindValue(0, astringId);
    if (!query.exec())
        throw 0;

    Q3Prediction q3;
    if (query.next())
    {
        q3.q3_ = query.value(0).toByteArray();
        q3.confidence_ = Q3Prediction::decodeConfidence(query.value(1).toString());
    }

    query.finish();

    return q3;
}

/**
  * @param astringId [int]
  * @param coils [QVector<Coil> &]
  */
void DbAstringCrud::saveCoils(int astringId, QVector<Coil> &coils) const
{
    // Step A: Build list of ids to keep
    QString validIds;
    foreach (const Coil &coil, coils)
    {
        if (coil.id_ > 0)
        {
            if (!validIds.isEmpty())
                validIds += QString(",%1").arg(coil.id_);
            else
                validIds = QString::number(coil.id_);
        }
    }

    // Step B: Delete these sequences from the database that are not associated with this object
    if (validIds.isEmpty() == false)
    {
        QSqlQuery query(dbSource()->database());
        if (!query.exec(QString("DELETE FROM coils WHERE astring_id = %1 AND NOT id IN (%2)").arg(astringId).arg(validIds)))
            throw 0;
        query.finish();
    }
    else
    {
        // Remove all coils
        QSqlQuery query = dbSource()->getPreparedQuery("deleteCoilsForAstring",
                                                       "DELETE FROM coils WHERE astring_id = ?");

        query.bindValue(0, astringId);
        if (!query.exec())
            throw 0;
        query.finish();
    }

    // Step C: Insert the new coil pods
    QSqlQuery insert = dbSource()->getPreparedQuery("insertCoil",
                                                    "INSERT INTO coils (astring_id, start, stop) "
                                                    "VALUES (?, ?, ?)");

    QVector<Coil>::Iterator it = coils.begin();
    for (; it != coils.end(); ++it)
    {
        Coil &coil = *it;
        if (coil.id_ != 0)  // Skip coils that already have an id
            continue;

        insert.bindValue(0, astringId);
        insert.bindValue(1, coil.begin());
        insert.bindValue(2, coil.end());
        if (!insert.exec())
            throw 0;

        coil.id_ = insert.lastInsertId().toInt();
    }

    insert.finish();
}

/**
  * @param astringId [int]
  * @param segs [QVector<Seg> &]
  */
void DbAstringCrud::saveSegs(int astringId, QVector<Seg> &segs) const
{
    // Step A: Build list of ids to keep
    QString validIds;
    foreach (const Seg &seg, segs)
    {
        if (seg.id_ > 0)
        {
            if (!validIds.isEmpty())
                validIds += QString(",%1").arg(seg.id_);
            else
                validIds = QString::number(seg.id_);
        }
    }

    // Step B: Delete these sequences from the database that are not associated with this object
    if (validIds.isEmpty() == false)
    {
        QSqlQuery query(dbSource()->database());
        if (!query.exec(QString("DELETE FROM segs WHERE astring_id = %1 AND NOT id IN (%2)").arg(astringId).arg(validIds)))
            throw 0;
        query.finish();
    }
    else
    {
        // Remove all coils
        QSqlQuery query = dbSource()->getPreparedQuery("deleteSegsForAstring",
                                                       "DELETE FROM segs WHERE astring_id = ?");

        query.bindValue(0, astringId);
        if (!query.exec())
            throw 0;
        query.finish();
    }

    // Step C: Insert the new coil pods
    QSqlQuery insert = dbSource()->getPreparedQuery("insertSeg",
                                                    "INSERT INTO segs(astring_id, start, stop) "
                                                    "VALUES (?, ?, ?)");

    QVector<Seg>::Iterator it = segs.begin();
    for (; it != segs.end(); ++it)
    {
        Seg &seg = *it;
        if (seg.id_ != 0)   // Skip segs that already have an id - these are already in the database
            continue;

        insert.bindValue(0, astringId);
        insert.bindValue(1, seg.begin());
        insert.bindValue(2, seg.end());
        if (!insert.exec())
            throw 0;

        seg.id_ = insert.lastInsertId().toInt();
    }

    insert.finish();
}

/**
  * @param astringId [int]
  * @param q3 [const Q3Prediction &]
  */
void DbAstringCrud::saveQ3(int astringId, const Q3Prediction &q3) const
{
    // Two cases:
    // 1) q3 is empty and therefore needs to be deleted
    // 2) q3 is non-empty and either needs to be inserted or updated

    // First, because both of the above cases involve a delete, we will do that every time
    QSqlQuery deleteQ3 = dbSource()->getPreparedQuery("saveQ3-delete",
                                                      "DELETE FROM q3 "
                                                      "WHERE astring_id = ?");

    deleteQ3.bindValue(0, astringId);
    if (!deleteQ3.exec())
        throw 0;

    deleteQ3.finish();

    // Second, attempt to insert the Q3 prediction
    if (!q3.isEmpty())
    {
        QSqlQuery insertQ3 = dbSource()->getPreparedQuery("saveQ3-insert",
                                                          "INSERT INTO q3(astring_id, q3, confidence) "
                                                          "VALUES (?, ?, ?)");
        insertQ3.bindValue(0, astringId);
        insertQ3.bindValue(1, q3.q3_);
        insertQ3.bindValue(2, q3.encodeConfidence());

        if (!insertQ3.exec())
            throw 0;

        insertQ3.finish();
    }
}
