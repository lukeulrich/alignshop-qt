/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "DbBlastReportCrud.h"
#include "../IDbSource.h"
#include "../../Entities/BlastReport.h"
#include "../../BioString.h"
#include "../../global.h"
#include "../../macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param dbSource [IDbSource *]
  */
DbBlastReportCrud::DbBlastReportCrud(IDbSource *dbSource)
    : AbstractDbEntityCrud<BlastReport, BlastReportPod>(dbSource)
{
    ASSERT(dbSource);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param blastReports [const QVector<BlastReport *> &]
  */
void DbBlastReportCrud::erase(const QVector<BlastReport *> &blastReports)
{
    QSqlQuery query = dbSource()->getPreparedQuery("eraseBlastReport",
                                                   "DELETE FROM blast_reports "
                                                   "WHERE id = ?");

    foreach (const BlastReport *blastReport, blastReports)
    {
        if (blastReport == nullptr)
            continue;

        query.bindValue(0, blastReport->id());
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
void DbBlastReportCrud::eraseByIds(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("eraseBlastReport",
                                                   "DELETE FROM blast_reports "
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
  * @returns QVector<BlastReportPod>
  */
QVector<BlastReportPod> DbBlastReportCrud::read(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("readBlastReports",
                                                   "SELECT id, name, description, notes, query_type, query_seq_id, "
                                                   "       query_start, query_stop, query_sequence, source_file, "
                                                   "       database, letters, sequences, bytes "
                                                   "FROM blast_reports "
                                                   "WHERE id = ? AND "
                                                   "    query_type IN ('amino', 'dna') AND "
                                                   "    query_start > 0 AND "
                                                   "    query_stop >= query_start AND "
                                                   "    length(source_file) > 0 AND "
                                                   "    sequences >= 0 AND "
                                                   "    letters >= 0 AND "
                                                   "    bytes >= 0");

    // --------------------------
    // --------------------------
    QVector<BlastReportPod> blastReportPods;
    blastReportPods.reserve(ids.size());
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
            blastReportPods << BlastReportPod();
            continue;
        }

        blastReportPods << BlastReportPod(query.value(0).toInt());
        BlastReportPod &pod = blastReportPods.last();
        pod.name_ = query.value(1).toString();
        pod.description_ = query.value(2).toString();
        pod.notes_ = query.value(3).toString();
        QString query_type = query.value(4).toString();
        if (query_type == "amino")
            pod.query_ = BioString(query.value(8).toByteArray(), eAminoGrammar);
        else
            pod.query_ = BioString(query.value(8).toByteArray(), eDnaGrammar);
        pod.querySeqId_ = query.value(5).toInt();
        pod.queryRange_ = ClosedIntRange(query.value(6).toInt(), query.value(7).toInt());
        pod.sourceFile_ = query.value(9).toString();

        pod.databaseSpec_ = BlastDatabaseSpec((query_type == "amino") ? eTrue : eFalse,
                                              query.value(10).toString(),
                                              query.value(11).toLongLong(),
                                              query.value(12).toLongLong(),
                                              query.value(13).toLongLong());
    }

    query.finish();

    return blastReportPods;
}

/**
  * @param blastReports [const QVector<BlastReport *> &]
  */
void DbBlastReportCrud::save(const QVector<BlastReport *> &blastReports)
{
    foreach (BlastReport *blastReport, blastReports)
    {
        if (!blastReport->isNew())
        {
            if (blastReport->isDirty())
                update(blastReport);
        }
        else
        {
            insert(blastReport);
        }
    }
}

/**
  * @param ids [const QVector<int> &]
  * @returns QStringList
  */
QStringList DbBlastReportCrud::sourceFiles(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("selectSourceFiles",
                                                   "SELECT source_file "
                                                   "FROM blast_reports "
                                                   "WHERE id = ?");

    QStringList sourceFiles;
    sourceFiles.reserve(ids.size());
    foreach (const int id, ids)
    {
        query.bindValue(0, id);
        if (!query.exec())
        {
            qDebug() << Q_FUNC_INFO << query.lastError().text();
            throw 0;
        }

        if (query.next())
            sourceFiles << query.value(0).toString();
    }

    query.finish();

    return sourceFiles;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param blastReport [BlastReport *]
  */
void DbBlastReportCrud::insert(BlastReport *blastReport)
{
    ASSERT(blastReport->isNew());
    QSqlQuery query = dbSource()->getPreparedQuery("insertBlastReport",
                                                   "INSERT INTO blast_reports ( "
                                                   "    name, description, notes, query_type, query_seq_id, "
                                                   "    query_start, query_stop, query_sequence, source_file, "
                                                   "    database, sequences, letters, bytes) "
                                                   "VALUES (?, ?, ?, ?, ?, "
                                                   "        ?, ?, ?, ?, "
                                                   "        ?, ?, ?, ?)");

    query.bindValue(0, blastReport->name());
    query.bindValue(1, blastReport->description());
    query.bindValue(2, blastReport->notes());
    ASSERT(blastReport->query().grammar() == eAminoGrammar ||
           blastReport->query().grammar() == eDnaGrammar);
    if (blastReport->query().grammar() == eAminoGrammar)
        query.bindValue(3, "amino");
    else
        query.bindValue(3, "dna");
    query.bindValue(4, blastReport->querySeqId());
    query.bindValue(5, blastReport->queryRange().begin_);
    query.bindValue(6, blastReport->queryRange().end_);
    query.bindValue(7, blastReport->query().asByteArray());
    query.bindValue(8, blastReport->sourceFile());
    query.bindValue(9, blastReport->databaseSpec().databaseName_);
    query.bindValue(10, blastReport->databaseSpec().nSequences_);
    query.bindValue(11, blastReport->databaseSpec().nLetters_);
    query.bindValue(12, blastReport->databaseSpec().nBytes_);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    blastReport->setId(query.lastInsertId().toInt());

    query.finish();
}

/**
  * @param blastReport [BlastReport *]
  */
void DbBlastReportCrud::update(BlastReport *blastReport)
{
    QSqlQuery query = dbSource()->getPreparedQuery("updateBlastReport",
                                                   "UPDATE blast_reports "
                                                   "SET name = ?, description = ?, notes = ?, query_type = ?, "
                                                   "    query_seq_id = ?, query_start = ?, query_stop = ?, "
                                                   "    query_sequence = ?, source_file = ?, "
                                                   "    database = ?, sequences = ?, letters = ?, bytes = ?"
                                                   "WHERE id = ?");

    query.bindValue(0, blastReport->name());
    query.bindValue(1, blastReport->description());
    query.bindValue(2, blastReport->notes());
    ASSERT(blastReport->query().grammar() == eAminoGrammar ||
           blastReport->query().grammar() == eDnaGrammar);
    if (blastReport->query().grammar() == eAminoGrammar)
        query.bindValue(3, "amino");
    else
        query.bindValue(3, "dna");
    query.bindValue(4, blastReport->querySeqId());
    query.bindValue(5, blastReport->queryRange().begin_);
    query.bindValue(6, blastReport->queryRange().end_);
    query.bindValue(7, blastReport->query().asByteArray());
    query.bindValue(8, blastReport->sourceFile());
    query.bindValue(9, blastReport->id());
    query.bindValue(10, blastReport->databaseSpec().databaseName_);
    query.bindValue(11, blastReport->databaseSpec().nSequences_);
    query.bindValue(12, blastReport->databaseSpec().nLetters_);
    query.bindValue(13, blastReport->databaseSpec().nBytes_);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    query.finish();
}
