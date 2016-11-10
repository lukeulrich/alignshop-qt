/****************************************************************************

**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVariant>
#include <QtSql/QSqlError>

#include "DbAminoMsaCrud.h"

#include "../IDbSource.h"
#include "../../Entities/AminoSeq.h"
#include "../../Entities/AminoMsa.h"
#include "../../Entities/EntityFlags.h"
#include "../../global.h"
#include "../../macros.h"
#include "../../Msa.h"
#include "../../ObservableMsa.h"
#include "../../Subseq.h"

#include <QtDebug>

class AminoSeq;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param dbSource [IDbSource *]
  */
DbAminoMsaCrud::DbAminoMsaCrud(IDbSource *dbSource) : AbstractDbEntityCrud<AminoMsa, AminoMsaPod>(dbSource)
{
    ASSERT(dbSource);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param aminoMsas [const QVector<AminoMsa *> &]
  */
void DbAminoMsaCrud::erase(const QVector<AminoMsa *> &aminoMsas)
{
    QSqlQuery eraseAminoSeqs = dbSource()->getPreparedQuery("DbAminoMsaCrud::erase-eraseAminoSeqs",
                                                            "DELETE FROM amino_seqs "
                                                            "WHERE id IN ( "
                                                            "    SELECT amino_seq_id "
                                                            "    FROM amino_msas_members "
                                                            "    WHERE amino_msa_id = ?)");

    QSqlQuery eraseAminoMsa = dbSource()->getPreparedQuery("DbAminoMsaCrud::erase-eraseAminoMsa",
                                                           "DELETE FROM amino_msas "
                                                           "WHERE id = ?");

    // --------------------------
    // --------------------------
    foreach (const AminoMsa *aminoMsa, aminoMsas)
    {
        if (aminoMsa == nullptr)
            continue;

        eraseAminoSeqs.bindValue(0, aminoMsa->id());
        if (!eraseAminoSeqs.exec())
        {
            qDebug() << Q_FUNC_INFO << eraseAminoSeqs.lastError().text();
            throw 0;
        }

        eraseAminoMsa.bindValue(0, aminoMsa->id());
        if (!eraseAminoMsa.exec())
        {
            qDebug() << Q_FUNC_INFO << eraseAminoMsa.lastError().text();
            throw 0;
        }
    }

    eraseAminoSeqs.finish();
    eraseAminoMsa.finish();
}

/**
  * @param ids [const QVector<int> &]
  */
void DbAminoMsaCrud::eraseByIds(const QVector<int> &ids)
{
    QSqlQuery eraseAminoSeqs = dbSource()->getPreparedQuery("DbAminoMsaCrud::eraseByIds-eraseAminoSeqs",
                                                            "DELETE FROM amino_seqs "
                                                            "WHERE id IN ( "
                                                            "    SELECT amino_seq_id "
                                                            "    FROM amino_msas_members "
                                                            "    WHERE amino_msa_id = ?)");

    QSqlQuery eraseAminoMsa = dbSource()->getPreparedQuery("DbAminoMsaCrud::eraseByIds-eraseAminoMsa",
                                                           "DELETE FROM amino_msas "
                                                           "WHERE id = ?");

    // --------------------------
    // --------------------------
    foreach (int id, ids)
    {
        eraseAminoSeqs.bindValue(0, id);
        if (!eraseAminoSeqs.exec())
        {
            qDebug() << Q_FUNC_INFO << eraseAminoSeqs.lastError().text();
            throw 0;
        }

        eraseAminoMsa.bindValue(0, id);
        if (!eraseAminoMsa.exec())
        {
            qDebug() << Q_FUNC_INFO << eraseAminoMsa.lastError().text();
            throw 0;
        }
    }

    eraseAminoSeqs.finish();
    eraseAminoMsa.finish();
}

/**
  * Only reads the annotation information and sequence count. The alignment data is loaded separately.
  *
  * @param ids [const QVector<int> &]
  * @returns QVector<AminoMsaPod>
  */
QVector<AminoMsaPod> DbAminoMsaCrud::read(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("DbAminoMsaCrud::read",
                                                   "SELECT id, name, description, notes "
                                                   "FROM amino_msas "
                                                   "WHERE id = ?");
    // --------------------------
    // --------------------------
    QVector<AminoMsaPod> aminoMsaPods;
    aminoMsaPods.reserve(ids.size());
    foreach (int id, ids)
    {
        query.bindValue(0, id);
        if (!query.exec())
        {
            qDebug() << Q_FUNC_INFO << query.lastError().text();
            throw 0;
        }

        if (!query.next())
        {
            aminoMsaPods << AminoMsaPod();
            continue;
        }

        aminoMsaPods << AminoMsaPod(query.value(0).toInt());
        AminoMsaPod &pod = aminoMsaPods.last();
        pod.name_ = query.value(1).toString();
        pod.description_ = query.value(2).toString();
        pod.notes_ = query.value(3).toString();
    }

    query.finish();

    return aminoMsaPods;
}

/**
  * @param aminoMsas [const QVector<AminoMsa *> &]
  */
void DbAminoMsaCrud::save(const QVector<AminoMsa *> &aminoMsas)
{
    foreach (AminoMsa *aminoMsa, aminoMsas)
    {
        if (aminoMsa == nullptr)
            continue;

        if (!aminoMsa->isNew())
            update(aminoMsa);
        else
            insert(aminoMsa);

        aminoMsa->setDirty(Ag::eCoreDataFlag, false);
    }
}

/**
  * @param aminoMsaId [int]
  * @returns int
  */
int DbAminoMsaCrud::countMembers(int aminoMsaId)
{
    QSqlQuery query = dbSource()->getPreparedQuery("DbAminoMsaCrud::countMembers",
                                                   "SELECT count(*) "
                                                   "FROM amino_msas_members "
                                                   "WHERE amino_msa_id = ?");

    query.bindValue(0, aminoMsaId);
    if (!query.exec() || !query.next())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    int amount = query.value(0).toInt();

    query.finish();

    return amount;
}

/**
  * A negative limit returns all rows.
  *
  * @param msaId [int]
  * @param offset [int]
  * @param limit [int]
  * @returns MsaMembersPod
  */
MsaMembersPod DbAminoMsaCrud::readMsaMembers(int msaId, int offset, int limit)
{
    QSqlQuery query = dbSource()->getPreparedQuery("DbAminoMsaCrud::readMsaMembers",
                                                   "SELECT amino_seq_id, sequence "
                                                   "FROM amino_msas_members "
                                                   "WHERE amino_msa_id = ? "
                                                   "ORDER BY position ASC "
                                                   "LIMIT ? "
                                                   "OFFSET ?");

    query.bindValue(0, msaId);
    query.bindValue(1, limit);
    query.bindValue(2, offset);

    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    MsaMembersPod pod;
    if (limit > 0)
    {
        pod.seqIds_.reserve(limit);
        pod.gappedSequences_.reserve(limit);
    }
    else if (limit < 0)
    {
        // Fetching all rows, count the members so that we can reserve space in the vector
        int memberCount = countMembers(msaId);
        pod.seqIds_.reserve(memberCount);
        pod.gappedSequences_.reserve(memberCount);
    }

    while (query.next())
    {
        pod.seqIds_ << query.value(0).toInt();
        pod.gappedSequences_ << query.value(1).toByteArray();
    }

    query.finish();

    return pod;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param aminoMsa [AminoMsa *]
  */
void DbAminoMsaCrud::insert(AminoMsa *aminoMsa) const
{
    insertCoreAminoMsa(aminoMsa);
    insertAminoMsaMembers(aminoMsa);
}

/**
  * @param aminoMsa [const AminoMsa *]
  */
void DbAminoMsaCrud::insertCoreAminoMsa(AminoMsa *aminoMsa) const
{
    ASSERT(aminoMsa != nullptr);
    ASSERT(aminoMsa->isNew());

    QSqlQuery insert = dbSource()->getPreparedQuery("DbAminoMsaCrud::insertCoreAminoMsa",
                                                    "INSERT INTO amino_msas (name, description, notes) "
                                                    "VALUES (?, ?, ?)");

    insert.bindValue(0, aminoMsa->name());
    insert.bindValue(1, aminoMsa->description());
    insert.bindValue(2, aminoMsa->notes());

    if (!insert.exec())
    {
        qDebug() << Q_FUNC_INFO << insert.lastError().text();
        throw 0;
    }

    aminoMsa->setId(insert.lastInsertId().toInt());

    insert.finish();
}

/**
  * @param aminoMsa [const AminoMsa *]
  */
void DbAminoMsaCrud::insertAminoMsaMembers(const AminoMsa *aminoMsa) const
{
    QSqlQuery insert =
        dbSource()->getPreparedQuery("DbAminoMsaCrud::insertAminoMsaMembers",
                                     "INSERT INTO amino_msas_members (amino_msa_id, amino_seq_id, position, sequence) "
                                     "VALUES (?, ?, ?, ?)");

    Msa *msa = aminoMsa->msa();
    if (msa == nullptr)
        return;

    insert.bindValue(0, aminoMsa->id());

    for (int i=0, z= msa->subseqCount(); i<z; ++i)
    {
        const Subseq *subseq = msa->at(i+1);
        IEntity *entity = subseq->seqEntity_.get();
        ASSERT(entity != nullptr);
        ASSERT(dynamic_cast<const AminoSeq *>(entity) != 0);

        insert.bindValue(1, entity->id());
        insert.bindValue(2, i+1);               // Position index
        insert.bindValue(3, subseq->asByteArray());

        if (!insert.exec())
        {
            qDebug() << Q_FUNC_INFO << insert.lastError().text();
            throw 0;
        }
    }

    insert.finish();
}

/**
  * @param aminoMsa [const AminoMsa *]
  */
void DbAminoMsaCrud::update(const AminoMsa *aminoMsa) const
{
    updateCoreAminoMsa(aminoMsa);
    if (aminoMsa->msa() != nullptr)
    {
        deleteOldAminoMsaMembers(aminoMsa);
        insertAminoMsaMembers(aminoMsa);
    }
}

/**
  * @param aminoMsa [const AminoMsa *]
  */
void DbAminoMsaCrud::updateCoreAminoMsa(const AminoMsa *aminoMsa) const
{
    ASSERT(aminoMsa != nullptr);

    QSqlQuery query = dbSource()->getPreparedQuery("DbAminoMsaCrud::updateCoreAminoMsa",
                                                   "UPDATE amino_msas "
                                                   "SET name = ?, description = ?, notes = ? "
                                                   "WHERE id = ?");

    query.bindValue(0, aminoMsa->name());
    query.bindValue(1, aminoMsa->description());
    query.bindValue(2, aminoMsa->notes());
    query.bindValue(3, aminoMsa->id());

    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    query.finish();
}


/**
  * @param aminoMsa [const AminoMsa *]
  */
void DbAminoMsaCrud::deleteOldAminoMsaMembers(const AminoMsa *aminoMsa) const
{
    ASSERT(aminoMsa != nullptr);

    QSqlQuery query = dbSource()->getPreparedQuery("DbAminoMsaCrud::deleteAminoMsaMembers",
                                                   "DELETE FROM amino_msas_members "
                                                   "WHERE amino_msa_id = ?");

    query.bindValue(0, aminoMsa->id());
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }
    query.finish();
}


