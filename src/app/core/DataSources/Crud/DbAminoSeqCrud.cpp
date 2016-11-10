/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVariant>
#include <QtSql/QSqlError>

#include "DbAminoSeqCrud.h"

#include "../IDbSource.h"
#include "../../Entities/AbstractAnonSeq.h"
#include "../../Entities/AminoSeq.h"
#include "../../Entities/EntityFlags.h"
#include "../../global.h"
#include "../../macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param dbSource [IDbSource *]
  */
DbAminoSeqCrud::DbAminoSeqCrud(IDbSource *dbSource) : AbstractDbEntityCrud<AminoSeq, AminoSeqPod>(dbSource)
{
    ASSERT(dbSource);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param aminoSeqs [const QVector<AminoSeq *> &]
  */
void DbAminoSeqCrud::save(const QVector<AminoSeq *> &aminoSeqs)
{
    foreach (AminoSeq *aminoSeq, aminoSeqs)
    {
        if (!aminoSeq->isNew())
        {
            if (aminoSeq->isDirty())
            {
                update(aminoSeq);
                aminoSeq->setDirty(Ag::eCoreDataFlag, false);
            }
        }
        else
        {
            insert(aminoSeq);
        }
    }
}

/**
  * @param aminoSeqs [const QVector<AminoSeq *> &]
  */
void DbAminoSeqCrud::erase(const QVector<AminoSeq *> &aminoSeqs)
{
    QSqlQuery query = dbSource()->getPreparedQuery("eraseAminoSeq",
                                                   "DELETE FROM amino_seqs "
                                                   "WHERE id = ?");

    foreach (const AminoSeq *aminoSeq, aminoSeqs)
    {
        if (aminoSeq == nullptr)
            continue;

        query.bindValue(0, aminoSeq->id());
        if (!query.exec())
        {
            qDebug() << Q_FUNC_INFO << query.lastError().text();
            throw 0;
        }
    }

    query.finish();
}

/**
  * @param ids [const QVector<int> &]
  */
void DbAminoSeqCrud::eraseByIds(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("eraseAminoSeq",
                                                   "DELETE FROM amino_seqs "
                                                   "WHERE id = ?");

    foreach (int id, ids)
    {
        query.bindValue(0, id);
        if (!query.exec())
        {
            qDebug() << Q_FUNC_INFO << query.lastError().text();
            throw 0;
        }
    }

    query.finish();
}

/**
  * @param ids [const QVector<int> &]
  * @returns QVector<AminoSeqPod>
  */
QVector<AminoSeqPod> DbAminoSeqCrud::read(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("readAminoSeqs",
                                                   "SELECT a.id, astring_id, start, stop, name, source, description, "
                                                   "       notes "
                                                   "FROM amino_seqs a JOIN astrings b ON (a.astring_id = b.id) "
                                                   "WHERE a.id = ? AND "
                                                   "    start > 0 AND "
                                                   "    stop >= start AND "
                                                   "    stop <= length(b.sequence);");

    // --------------------------
    // --------------------------
    QVector<AminoSeqPod> aminoSeqPods;
    aminoSeqPods.reserve(ids.size());
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
            aminoSeqPods << AminoSeqPod();
            continue;
        }

        aminoSeqPods << AminoSeqPod(query.value(0).toInt());
        AminoSeqPod &pod = aminoSeqPods.last();
        pod.start_ = query.value(2).toInt();
        pod.stop_ = query.value(3).toInt();
        pod.name_ = query.value(4).toString();
        pod.source_ = query.value(5).toString();
        pod.description_ = query.value(6).toString();
        pod.notes_ = query.value(7).toString();

        // Note, we do not assign the astring_ pointer because we do not know anything about creating the relevant
        // astring. Thus, we simply pass the identifier of the associated astring into the constructor and leave the job
        // of associating / finding the relevant astring to the data mapper.
        pod.astringId_ = query.value(1).toInt();
    }

    query.finish();

    return aminoSeqPods;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @aminoSeq [AminoSeq *]
  */
void DbAminoSeqCrud::insert(AminoSeq *aminoSeq)
{
    ASSERT(aminoSeq->isNew());
    QSqlQuery insert = dbSource()->getPreparedQuery("insertAminoSeq",
                                                    "INSERT INTO amino_seqs (astring_id, start, stop, name, source, "
                                                    "                        description, notes) "
                                                    "VALUES (?, ?, ?, ?, ?, ?, ?)");

    insert.bindValue(0, aminoSeq->abstractAnonSeq()->id());
    insert.bindValue(1, aminoSeq->start());
    insert.bindValue(2, aminoSeq->stop());
    insert.bindValue(3, aminoSeq->name());
    insert.bindValue(4, aminoSeq->source());
    insert.bindValue(5, aminoSeq->description());
    insert.bindValue(6, aminoSeq->notes());
    if (!insert.exec())
    {
        qDebug() << Q_FUNC_INFO << insert.lastError().text();
        throw 0;
    }

    aminoSeq->setId(insert.lastInsertId().toInt());

    insert.finish();
}

/**
  * @aminoSeq [AminoSeq *]
  */
void DbAminoSeqCrud::update(const AminoSeq *aminoSeq)
{
    QSqlQuery update = dbSource()->getPreparedQuery("updateAminoSeq",
                                                   "UPDATE amino_seqs "
                                                   "SET start = ?, stop = ?, name = ?, source = ?, description = ?, "
                                                   "    notes = ? "
                                                   "WHERE id = ?");

    update.bindValue(0, aminoSeq->start());
    update.bindValue(1, aminoSeq->stop());
    update.bindValue(2, aminoSeq->name());
    update.bindValue(3, aminoSeq->source());
    update.bindValue(4, aminoSeq->description());
    update.bindValue(5, aminoSeq->notes());
    update.bindValue(6, aminoSeq->id());
    if (!update.exec())
    {
        qDebug() << Q_FUNC_INFO << update.lastError().text();
        throw 0;
    }

    update.finish();
}
