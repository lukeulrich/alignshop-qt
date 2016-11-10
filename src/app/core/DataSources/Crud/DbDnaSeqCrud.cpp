/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVariant>

#include <QtSql/QSqlError>

#include "DbDnaSeqCrud.h"

#include "DbPrimerSearchParametersCache.h"
#include "../IDbSource.h"
#include "../../Entities/AbstractAnonSeq.h"
#include "../../Entities/DnaSeq.h"
#include "../../Entities/EntityFlags.h"
#include "../../../primer/PrimerFactory.h"
#include "../../global.h"
#include "../../macros.h"


#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param dbSource [IDbSource *]
  */
DbDnaSeqCrud::DbDnaSeqCrud(IDbSource *dbSource) : AbstractDbEntityCrud<DnaSeq, DnaSeqPod>(dbSource)
{
    ASSERT(dbSource);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param dnaSeqs [const QVector<DnaSeq *> &]
  */
void DbDnaSeqCrud::save(const QVector<DnaSeq *> &dnaSeqs)
{
    foreach (DnaSeq *dnaSeq, dnaSeqs)
    {
        if (!dnaSeq->isNew())
            update(dnaSeq);
        else
            insert(dnaSeq);
    }
}

/**
  * @param dnaSeqs [const QVector<DnaSeq *> &]
  */
void DbDnaSeqCrud::erase(const QVector<DnaSeq *> &dnaSeqs)
{
    QSqlQuery query = dbSource()->getPreparedQuery("eraseDnaSeq",
                                                   "DELETE FROM dna_seqs "
                                                   "WHERE id = ?");

    foreach (const DnaSeq *dnaSeq, dnaSeqs)
    {
        if (dnaSeq == nullptr)
            continue;

        erasePrimers(dnaSeq->id());

        query.bindValue(0, dnaSeq->id());
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
void DbDnaSeqCrud::eraseByIds(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("eraseDnaSeq",
                                                   "DELETE FROM dna_seqs "
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
  * @returns QVector<DnaSeqPod>
  */
QVector<DnaSeqPod> DbDnaSeqCrud::read(const QVector<int> &ids)
{
    QSqlQuery query = dbSource()->getPreparedQuery("readDnaSeqs",
                                                   "SELECT a.id, dstring_id, start, stop, name, source, description, "
                                                   "       notes "
                                                   "FROM dna_seqs a JOIN dstrings b ON (a.dstring_id = b.id) "
                                                   "WHERE a.id = ? AND "
                                                   "    start > 0 AND "
                                                   "    stop >= start AND "
                                                   "    stop <= length(b.sequence);");

    // --------------------------
    // --------------------------
    QVector<DnaSeqPod> dnaSeqPods;
    dnaSeqPods.reserve(ids.size());
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
            dnaSeqPods << DnaSeqPod();
            continue;
        }

        int dnaSeqId = query.value(0).toInt();
        dnaSeqPods << DnaSeqPod(dnaSeqId);
        DnaSeqPod &pod = dnaSeqPods.last();
        pod.start_ = query.value(2).toInt();
        pod.stop_ = query.value(3).toInt();
        pod.name_ = query.value(4).toString();
        pod.source_ = query.value(5).toString();
        pod.description_ = query.value(6).toString();
        pod.notes_ = query.value(7).toString();

        pod.primers_ = readPrimers(dnaSeqId);

        // Note, we do not assign the dstring_ pointer because we do not know anything about creating the relevant
        // dstring. Thus, we simply pass the identifier of the associated dstring into the constructor and leave the job
        // of associating / finding the relevant dstring to the data mapper.
        pod.dstringId_ = query.value(1).toInt();
    }

    query.finish();

    return dnaSeqPods;
}

void DbDnaSeqCrud::erasePrimers(const int dnaSeqId)
{
    QSqlQuery query = dbSource()->getPreparedQuery("eraseDnaSeqPrimers",
                                                   "DELETE FROM primers "
                                                   "WHERE id IN ("
                                                   "   SELECT id FROM dna_seqs_primers"
                                                   "   WHERE dna_seq_id = ?)");
    query.bindValue(0, dnaSeqId);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    query = dbSource()->getPreparedQuery("eraseDnaSeqPrimerAssociations",
                                         "DELETE FROM dna_seqs_primers "
                                         "WHERE dna_seq_id = ?");
    query.bindValue(0, dnaSeqId);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }
    query.finish();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
void DbDnaSeqCrud::update(DnaSeq *dnaSeq)
{
    ASSERT(!dnaSeq->isNew());
    if (dnaSeq->isDirty(Ag::eCoreDataFlag))
    {
        updateCoreDnaSeq(dnaSeq);
        dnaSeq->setDirty(Ag::eCoreDataFlag, false);
    }

    if (dnaSeq->isDirty(Ag::ePrimersFlag))
    {
        savePrimers(dnaSeq->id(), dnaSeq->primers_);
        dnaSeq->setDirty(Ag::ePrimersFlag, false);
    }
}

/**
  * @dnaSeq [DnaSeq *]
  */
void DbDnaSeqCrud::insert(DnaSeq *dnaSeq)
{
    ASSERT(dnaSeq->isNew());
    insertCoreDnaSeq(dnaSeq);
    savePrimers(dnaSeq->id(), dnaSeq->primers_);
}

void DbDnaSeqCrud::insertCoreDnaSeq(DnaSeq *dnaSeq)
{
    QSqlQuery query = dbSource()->getPreparedQuery("insertDnaSeq",
                                                   "INSERT INTO dna_seqs (dstring_id, start, stop, name, source, "
                                                   "                        description, notes) "
                                                   "VALUES (?, ?, ?, ?, ?, ?, ?)");

    query.bindValue(0, dnaSeq->abstractAnonSeq()->id());
    query.bindValue(1, dnaSeq->start());
    query.bindValue(2, dnaSeq->stop());
    query.bindValue(3, dnaSeq->name());
    query.bindValue(4, dnaSeq->source());
    query.bindValue(5, dnaSeq->description());
    query.bindValue(6, dnaSeq->notes());
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    dnaSeq->setId(query.lastInsertId().toInt());

    query.finish();
}

/**
  * @dnaSeq [DnaSeq *]
  */
void DbDnaSeqCrud::updateCoreDnaSeq(const DnaSeq *dnaSeq)
{
    QSqlQuery query = dbSource()->getPreparedQuery("updateDnaSeq",
                                                   "UPDATE dna_seqs "
                                                   "SET start = ?, stop = ?, name = ?, source = ?, description = ?, "
                                                   "    notes = ? "
                                                   "WHERE id = ?");
    query.bindValue(0, dnaSeq->start());
    query.bindValue(1, dnaSeq->stop());
    query.bindValue(2, dnaSeq->name());
    query.bindValue(3, dnaSeq->source());
    query.bindValue(4, dnaSeq->description());
    query.bindValue(5, dnaSeq->notes());
    query.bindValue(6, dnaSeq->id());
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    query.finish();
}

void DbDnaSeqCrud::savePrimers(int dnaSeqId, PrimerVector &primers)
{
    QVector<int> validPrimerIds = primerIdVector(primers);
    deleteDnaSeqPrimersNotIn(dnaSeqId, validPrimerIds);

    for (int i=0, z=primers.size(); i<z; ++i)
    {
        Primer &primer = primers[i];
        if (primer.isNew())
        {
            insertPrimer(primer);
            associatePrimerWithDnaSeq(dnaSeqId, primer.id());
        }
        else if (primer.isDirty())
        {
            updatePrimer(primer);
            primer.setDirty(Ag::eCoreDataFlag, false);
        }
    }
}

void DbDnaSeqCrud::insertPrimer(Primer &primer)
{
    QVariant pspId;
    // Check if there is a PSP to insert
    if (primer.primerSearchParameters() != nullptr)
    {
        if (primer.primerSearchParameters()->isNew())
        {
            pspId = insertPrimerSearchParameters(primer.primerSearchParameters());
            primer.setPrimerSearchParametersId(pspId.toInt());
        }
        else
        {
            // There is no updating of a primer search parameter object
            pspId = primer.primerSearchParameters()->id_;
        }
    }

    QSqlQuery query = dbSource()->getPreparedQuery("insertPrimer",
                                                   "INSERT INTO primers "
                                                   "    (primer_search_parameters_id, name, "
                                                   "     re_name, re_site, re_sense_cuts, re_antisense_cuts, "
                                                   "     core_sequence, tm, homo_dimer_score) "
                                                   "VALUES (?, ?, "
                                                   "        ?, ?, ?, ?, "
                                                   "        ?, ?, ?)");
    query.bindValue(0, pspId);
    query.bindValue(1, primer.name());
    query.bindValue(2, primer.restrictionEnzyme().name());
    query.bindValue(3, primer.restrictionEnzyme().recognitionSite().asByteArray());
    query.bindValue(4, RestrictionEnzyme::serializeCuts(primer.restrictionEnzyme().forwardCuts()));
    query.bindValue(5, RestrictionEnzyme::serializeCuts(primer.restrictionEnzyme().reverseCuts()));
    query.bindValue(6, primer.coreSequence().asByteArray());
    query.bindValue(7, primer.tm());
    query.bindValue(8, primer.homoDimerScore());
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    primer.setId(query.lastInsertId().toInt());

    query.finish();
}

int DbDnaSeqCrud::insertPrimerSearchParameters(const PrimerSearchParameters *primerSearchParameters)
{
    QSqlQuery query = dbSource()->getPreparedQuery("insertPrimerSearchParameters",
                                                   "INSERT INTO primer_search_parameters "
                                                   "    (amplicon_length_min, amplicon_length_max, primer_length_min, primer_length_max, "
                                                   "     forward_re_name, forward_re_site, forward_re_sense_cuts, forward_re_antisense_cuts, "
                                                   "     reverse_re_name, reverse_re_site, reverse_re_sense_cuts, reverse_re_antisense_cuts, "
                                                   "     forward_terminal_pattern, reverse_terminal_pattern, "
                                                   "     primer_tm_range_min, primer_tm_range_max, "
                                                   "     na_molar_concentration, primer_dna_molar_concentration, max_primer_pair_delta_tm) "
                                                   "VALUES (?, ?, ?, ?, "
                                                   "        ?, ?, ?, ?, "
                                                   "        ?, ?, ?, ?, "
                                                   "        ?, ?, "
                                                   "        ?, ?, "
                                                   "        ?, ?, ?)");

    query.bindValue(0, primerSearchParameters->ampliconLengthRange_.begin_);
    query.bindValue(1, primerSearchParameters->ampliconLengthRange_.end_);
    query.bindValue(2, primerSearchParameters->primerLengthRange_.begin_);
    query.bindValue(3, primerSearchParameters->primerLengthRange_.end_);
    query.bindValue(4, primerSearchParameters->forwardRestrictionEnzyme_.name());
    query.bindValue(5, primerSearchParameters->forwardRestrictionEnzyme_.recognitionSite().asByteArray());
    query.bindValue(6, RestrictionEnzyme::serializeCuts(primerSearchParameters->forwardRestrictionEnzyme_.forwardCuts()));
    query.bindValue(7, RestrictionEnzyme::serializeCuts(primerSearchParameters->forwardRestrictionEnzyme_.reverseCuts()));
    query.bindValue(8, primerSearchParameters->reverseRestrictionEnzyme_.name());
    query.bindValue(9, primerSearchParameters->reverseRestrictionEnzyme_.recognitionSite().asByteArray());
    query.bindValue(10, RestrictionEnzyme::serializeCuts(primerSearchParameters->reverseRestrictionEnzyme_.forwardCuts()));
    query.bindValue(11, RestrictionEnzyme::serializeCuts(primerSearchParameters->reverseRestrictionEnzyme_.reverseCuts()));
    query.bindValue(12, primerSearchParameters->forwardTerminalPattern_.pattern());
    query.bindValue(13, primerSearchParameters->reverseTerminalPattern_.pattern());
    query.bindValue(14, primerSearchParameters->individualPrimerTmRange_.begin_);
    query.bindValue(15, primerSearchParameters->individualPrimerTmRange_.end_);
    query.bindValue(16, primerSearchParameters->sodiumConcentration_);
    query.bindValue(17, primerSearchParameters->primerDnaConcentration_);
    query.bindValue(18, primerSearchParameters->maximumPrimerPairDeltaTm_);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    int id = query.lastInsertId().toInt();
    query.finish();
    return id;
}

void DbDnaSeqCrud::associatePrimerWithDnaSeq(int dnaSeqId, int primerId)
{
    QSqlQuery query = dbSource()->getPreparedQuery("insertDnaSeqsPrimers",
                                                   "INSERT INTO dna_seqs_primers "
                                                   "    (dna_seq_id, primer_id)"
                                                   "VALUES (?, ?)");
    query.bindValue(0, dnaSeqId);
    query.bindValue(1, primerId);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    query.finish();
}

void DbDnaSeqCrud::updatePrimer(Primer &primer)
{
    QSqlQuery query = dbSource()->getPreparedQuery("updatePrimer",
                                                   "UPDATE primers "
                                                   "SET primer_search_parameters_id = ?, name = ?, "
                                                   "    re_name = ?, re_site = ?, re_sense_cuts = ?, re_antisense_cuts = ?, "
                                                   "    core_sequence = ?, tm = ?, homo_dimer_score = ? "
                                                   "WHERE id = ?");
    QVariant pspId;
    if (primer.primerSearchParameters() != nullptr)
        pspId = primer.primerSearchParameters()->id_;
    query.bindValue(0, pspId);
    query.bindValue(1, primer.name());
    query.bindValue(2, primer.restrictionEnzyme().name());
    query.bindValue(3, primer.restrictionEnzyme().recognitionSite().asByteArray());
    query.bindValue(4, RestrictionEnzyme::serializeCuts(primer.restrictionEnzyme().forwardCuts()));
    query.bindValue(5, RestrictionEnzyme::serializeCuts(primer.restrictionEnzyme().reverseCuts()));
    query.bindValue(6, primer.coreSequence().asByteArray());
    query.bindValue(7, primer.tm());
    query.bindValue(8, primer.homoDimerScore());
    query.bindValue(9, primer.id());
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }
    query.finish();
}

QVector<int> DbDnaSeqCrud::primerIdVector(const PrimerVector &primers) const
{
    QVector<int> primerIds;
    foreach (const Primer &primer, primers)
        primerIds << primer.id();
    return primerIds;
}

void DbDnaSeqCrud::deleteDnaSeqPrimersNotIn(const int dnaSeqId, const QVector<int> &validPrimerIds)
{
    QStringList validPrimerIdStrings;
    foreach (const int i, validPrimerIds)
        validPrimerIdStrings << QString::number(i);
    QString joinedPrimerIdString = validPrimerIdStrings.join(",");

    deleteFromPrimersTable(dnaSeqId, joinedPrimerIdString);
    deleteFromDnaSeqPrimersTable(dnaSeqId, joinedPrimerIdString);
}

void DbDnaSeqCrud::deleteFromPrimersTable(const int dnaSeqId, const QString validPrimerIdString)
{
    QString deleteSql = QString("DELETE FROM primers "
                                "WHERE id IN ("
                                "   SELECT primer_id FROM dna_seqs_primers "
                                "   WHERE dna_seq_id = ? AND primer_id NOT IN (%1))").arg(validPrimerIdString);
    QSqlQuery query(dbSource()->database());
    if (!query.prepare(deleteSql))
    {
        qDebug() << query.lastError().text();
        throw 0;
    }
    query.bindValue(0, dnaSeqId);
    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        throw 0;
    }

    query.finish();
}

void DbDnaSeqCrud::deleteFromDnaSeqPrimersTable(const int dnaSeqId, const QString validPrimerIdString)
{
    QString deleteSql = QString("DELETE FROM dna_seqs_primers "
                                "WHERE dna_seq_id = ? AND NOT primer_id IN (%1)").arg(validPrimerIdString);
    QSqlQuery query(dbSource()->database());
    if (!query.prepare(deleteSql))
    {
        qDebug() << query.lastError().text();
        throw 0;
    }
    query.bindValue(0, dnaSeqId);
    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        throw 0;
    }

    query.finish();
}

PrimerVector DbDnaSeqCrud::readPrimers(const int dnaSeqId) const
{
    static DbPrimerSearchParametersCache *pspCache = new DbPrimerSearchParametersCache(dbSource());

    QVector<int> uniquePSPids = fetchUniquePrimerSearchParameterIds(dnaSeqId);
    pspCache->cacheRecords(uniquePSPids);

    QSqlQuery query = dbSource()->getPreparedQuery("readPrimers",
                                                   "SELECT b.id, b.primer_search_parameters_id, b.name, "
                                                   "       b.re_name, b.re_site, b.re_sense_cuts, b.re_antisense_cuts, "
                                                   "       b.core_sequence, b.tm, b.homo_dimer_score "
                                                   "FROM dna_seqs_primers a JOIN "
                                                   "     primers b ON (a.primer_id = b.id) "
                                                   "WHERE a.dna_seq_id = ?");

    query.bindValue(0, dnaSeqId);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    PrimerVector primers;
    while (query.next())
    {
        int primerId = query.value(0).toInt();
        QSharedPointer<PrimerSearchParameters> psp;
        if (!query.isNull(1))
            psp = pspCache->read(query.value(1).toInt());
        QString name = query.value(2).toString();
        QString REName = query.value(3).toString();
        BioString RESite = BioString(query.value(4).toByteArray(), eDnaGrammar);
        QVector<int> forwardCuts = RestrictionEnzyme::deserializeCutString(query.value(5).toString());
        QVector<int> reverseCuts = RestrictionEnzyme::deserializeCutString(query.value(6).toString());
        RestrictionEnzyme RE(REName, RESite, forwardCuts, reverseCuts);
        BioString coreSequence = BioString(query.value(7).toByteArray(), eDnaGrammar);
        double tm = query.value(8).toDouble();
        double homoDimerScore = query.value(9).toDouble();

        primers << Primer(primerId, coreSequence, RE, tm, homoDimerScore, psp);
        primers.last().setName(name);
    }
    query.finish();
    return primers;
}

QVector<int> DbDnaSeqCrud::fetchUniquePrimerSearchParameterIds(const int dnaSeqId) const
{
    QSqlQuery query = dbSource()->getPreparedQuery("readUniquePSPIds",
                                                   "SELECT distinct(c.id) "
                                                   "FROM dna_seqs_primers a JOIN "
                                                   "     primers b ON (a.primer_id = b.id) JOIN "
                                                   "     primer_search_parameters c ON (b.primer_search_parameters_id = c.id) "
                                                   "WHERE a.dna_seq_id = ?");
    query.bindValue(0, dnaSeqId);
    if(!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    QVector<int> uniquePSPids;
    while (query.next())
        uniquePSPids << query.value(0).toInt();
    query.finish();
    return uniquePSPids;
}
