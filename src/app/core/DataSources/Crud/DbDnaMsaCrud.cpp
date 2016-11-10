/****************************************************************************

**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVariant>
#include <QtSql/QSqlError>

#include "DbDnaMsaCrud.h"

#include "../IDbSource.h"
#include "../../Entities/DnaSeq.h"
#include "../../Entities/DnaMsa.h"
#include "../../Entities/EntityFlags.h"
#include "../../global.h"
#include "../../macros.h"
#include "../../Msa.h"
#include "../../ObservableMsa.h"
#include "../../Subseq.h"

#include <QtDebug>

class DnaSeq;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param dbSource [IDbSource *]
  */
DbDnaMsaCrud::DbDnaMsaCrud(IDbSource *dbSource) : AbstractDbEntityCrud<DnaMsa, DnaMsaPod>(dbSource)
{
    ASSERT(dbSource);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param dnaMsas [const QVector<DnaMsa *> &]
  */
void DbDnaMsaCrud::erase(const QVector<DnaMsa *> &dnaMsas)
{
    QSqlQuery eraseDnaSeqs = dbSource()->getPreparedQuery("DbDnaMsaCrud::erase-eraseDnaSeqs",
                                                          "DELETE FROM dna_seqs "
                                                          "WHERE id IN ( "
                                                          "    SELECT dna_seq_id "
                                                          "    FROM dna_msas_members "
                                                          "    WHERE dna_msa_id = ?)");

    QSqlQuery eraseDnaMsa = dbSource()->getPreparedQuery("DbDnaMsaCrud::erase-eraseDnaMsa",
                                                         "DELETE FROM dna_msas "
                                                         "WHERE id = ?");

    // --------------------------
    // --------------------------
    foreach (const DnaMsa *dnaMsa, dnaMsas)
    {
        if (dnaMsa == nullptr)
            continue;

        eraseDnaSeqs.bindValue(0, dnaMsa->id());
        if (!eraseDnaSeqs.exec())
        {
            qDebug() << Q_FUNC_INFO << eraseDnaSeqs.lastError().text();
            throw 0;
        }

        eraseDnaMsa.bindValue(0, dnaMsa->id());
        if (!eraseDnaMsa.exec())
        {
            qDebug() << Q_FUNC_INFO << eraseDnaMsa.lastError().text();
            throw 0;
        }
    }

    eraseDnaSeqs.finish();
    eraseDnaMsa.finish();
}

/**
  * @param ids [const QVector<int> &]
  */
void DbDnaMsaCrud::eraseByIds(const QVector<int> &ids)
{
    QSqlQuery eraseDnaSeqs = dbSource()->getPreparedQuery("DbDnaMsaCrud::eraseByIds-eraseDnaSeqs",
                                                          "DELETE FROM dna_seqs "
                                                          "WHERE id IN ( "
                                                          "    SELECT dna_seq_id "
                                                          "    FROM dna_msas_members "
                                                          "    WHERE dna_msa_id = ?)");

    QSqlQuery eraseDnaMsa = dbSource()->getPreparedQuery("DbDnaMsaCrud::eraseByIds-eraseDnaMsa",
                                                         "DELETE FROM dna_msas "
                                                         "WHERE id = ?");

    // --------------------------
    // --------------------------
    foreach (int id, ids)
    {
        eraseDnaSeqs.bindValue(0, id);
        if (!eraseDnaSeqs.exec())
        {
            qDebug() << Q_FUNC_INFO << eraseDnaSeqs.lastError().text();
            throw 0;
        }

        eraseDnaMsa.bindValue(0, id);
        if (!eraseDnaMsa.exec())
        {
            qDebug() << Q_FUNC_INFO << eraseDnaMsa.lastError().text();
            throw 0;
        }
    }

    eraseDnaSeqs.finish();
    eraseDnaMsa.finish();
}

/**
  * Only reads the annotation information and sequence count. The alignment data is loaded separately.
  *
  * @param ids [const QVector<int> &]
  * @returns QVector<DnaMsaPod>
  */
QVector<DnaMsaPod> DbDnaMsaCrud::read(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("DbDnaMsaCrud::read",
                                                   "SELECT id, name, description, notes "
                                                   "FROM dna_msas "
                                                   "WHERE id = ?");
    // --------------------------
    // --------------------------
    QVector<DnaMsaPod> dnaMsaPods;
    dnaMsaPods.reserve(ids.size());
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
            dnaMsaPods << DnaMsaPod();
            continue;
        }

        dnaMsaPods << DnaMsaPod(query.value(0).toInt());
        DnaMsaPod &pod = dnaMsaPods.last();
        pod.name_ = query.value(1).toString();
        pod.description_ = query.value(2).toString();
        pod.notes_ = query.value(3).toString();
    }

    query.finish();

    return dnaMsaPods;
}

/**
  * @param dnaMsas [const QVector<DnaMsa *> &]
  */
void DbDnaMsaCrud::save(const QVector<DnaMsa *> &dnaMsas)
{
    foreach (DnaMsa *dnaMsa, dnaMsas)
    {
        if (dnaMsa == nullptr)
            continue;

        if (!dnaMsa->isNew())
            update(dnaMsa);
        else
            insert(dnaMsa);

        dnaMsa->setDirty(Ag::eCoreDataFlag, false);
    }
}

/**
  * @param dnaMsaId [int]
  * @returns int
  */
int DbDnaMsaCrud::countMembers(int dnaMsaId)
{
    QSqlQuery query = dbSource()->getPreparedQuery("DbDnaMsaCrud::countMembers",
                                                   "SELECT count(*) "
                                                   "FROM dna_msas_members "
                                                   "WHERE dna_msa_id = ?");

    query.bindValue(0, dnaMsaId);
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
MsaMembersPod DbDnaMsaCrud::readMsaMembers(int msaId, int offset, int limit)
{
    QSqlQuery query = dbSource()->getPreparedQuery("DbDnaMsaCrud::readMsaMembers",
                                                   "SELECT dna_seq_id, sequence "
                                                   "FROM dna_msas_members "
                                                   "WHERE dna_msa_id = ? "
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
  * @param dnaMsa [DnaMsa *]
  */
void DbDnaMsaCrud::insert(DnaMsa *dnaMsa) const
{
    insertCoreDnaMsa(dnaMsa);
    insertDnaMsaMembers(dnaMsa);
}

/**
  * @param dnaMsa [const DnaMsa *]
  */
void DbDnaMsaCrud::insertCoreDnaMsa(DnaMsa *dnaMsa) const
{
    ASSERT(dnaMsa != nullptr);
    ASSERT(dnaMsa->isNew());

    QSqlQuery insert = dbSource()->getPreparedQuery("DbDnaMsaCrud::insertCoreDnaMsa",
                                                    "INSERT INTO dna_msas (name, description, notes) "
                                                    "VALUES (?, ?, ?)");

    insert.bindValue(0, dnaMsa->name());
    insert.bindValue(1, dnaMsa->description());
    insert.bindValue(2, dnaMsa->notes());

    if (!insert.exec())
    {
        qDebug() << Q_FUNC_INFO << insert.lastError().text();
        throw 0;
    }

    dnaMsa->setId(insert.lastInsertId().toInt());

    insert.finish();
}

/**
  * @param dnaMsa [const DnaMsa *]
  */
void DbDnaMsaCrud::insertDnaMsaMembers(const DnaMsa *dnaMsa) const
{
    QSqlQuery insert =
        dbSource()->getPreparedQuery("DbDnaMsaCrud::insertDnaMsaMembers",
                                     "INSERT INTO dna_msas_members (dna_msa_id, dna_seq_id, position, sequence) "
                                     "VALUES (?, ?, ?, ?)");

    Msa *msa = dnaMsa->msa();
    if (msa == nullptr)
        return;

    insert.bindValue(0, dnaMsa->id());

    for (int i=0, z= msa->subseqCount(); i<z; ++i)
    {
        const Subseq *subseq = msa->at(i+1);
        IEntity *entity = subseq->seqEntity_.get();
        ASSERT(entity != nullptr);
        ASSERT(dynamic_cast<const DnaSeq *>(entity) != 0);

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
  * @param dnaMsa [const DnaMsa *]
  */
void DbDnaMsaCrud::update(const DnaMsa *dnaMsa) const
{
    updateCoreDnaMsa(dnaMsa);
    if (dnaMsa->msa() != nullptr)
    {
        deleteOldDnaMsaMembers(dnaMsa);
        insertDnaMsaMembers(dnaMsa);
    }
}

/**
  * @param dnaMsa [const DnaMsa *]
  */
void DbDnaMsaCrud::updateCoreDnaMsa(const DnaMsa *dnaMsa) const
{
    ASSERT(dnaMsa != nullptr);

    QSqlQuery query = dbSource()->getPreparedQuery("DbDnaMsaCrud::updateCoreDnaMsa",
                                                   "UPDATE dna_msas "
                                                   "SET name = ?, description = ?, notes = ? "
                                                   "WHERE id = ?");

    query.bindValue(0, dnaMsa->name());
    query.bindValue(1, dnaMsa->description());
    query.bindValue(2, dnaMsa->notes());
    query.bindValue(3, dnaMsa->id());

    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    query.finish();
}


/**
  * @param dnaMsa [const DnaMsa *]
  */
void DbDnaMsaCrud::deleteOldDnaMsaMembers(const DnaMsa *dnaMsa) const
{
    ASSERT(dnaMsa != nullptr);

    QSqlQuery query = dbSource()->getPreparedQuery("DbDnaMsaCrud::deleteDnaMsaMembers",
                                                   "DELETE FROM dna_msas_members "
                                                   "WHERE dna_msa_id = ?");

    query.bindValue(0, dnaMsa->id());
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }
    query.finish();
}


