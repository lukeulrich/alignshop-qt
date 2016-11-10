/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <QtDebug>

#include "DbPrimerSearchParametersCache.h"
#include "../IDbSource.h"
#include "../../global.h"
#include "../../../primer/RestrictionEnzyme.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
DbPrimerSearchParametersCache::DbPrimerSearchParametersCache(IDbSource *dbSource)
    : dbSource_(dbSource)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void DbPrimerSearchParametersCache::clearCache()
{
    cachedRecords_.clear();
}

void DbPrimerSearchParametersCache::cacheRecords(const QVector<int> &primerSearchParametersIds)
{
    foreach (const int primerSearchParametersId, primerSearchParametersIds)
    {
        if (isCached(primerSearchParametersId))
            continue;

        QSharedPointer<PrimerSearchParameters> record = readFromDatabase(primerSearchParametersId);
        insertIntoCache(record);
    }
}

bool DbPrimerSearchParametersCache::isCached(const int primerSearchParametersId) const
{
    return cachedRecords_.contains(primerSearchParametersId);
}

QSharedPointer<PrimerSearchParameters> DbPrimerSearchParametersCache::read(const int primerSearchParameterId)
{
    if (isCached(primerSearchParameterId))
        return cachedRecords_.value(primerSearchParameterId);

    QSharedPointer<PrimerSearchParameters> record = readFromDatabase(primerSearchParameterId);
    insertIntoCache(record);
    return record;
}


// ------------------------------------------------------------------------------------------------
// Private methods
QSharedPointer<PrimerSearchParameters> DbPrimerSearchParametersCache::readFromDatabase(const int primerSearchParametersId) const
{
    if (dbSource_ == nullptr)
        return QSharedPointer<PrimerSearchParameters>();

    QSqlQuery query = dbSource_->getPreparedQuery("readPrimerSearchParametersRecord",
                                                  "SELECT id, amplicon_length_min, amplicon_length_max, "
                                                  "       primer_length_min, primer_length_max, "
                                                  "       forward_re_name, forward_re_site, forward_re_sense_cuts, forward_re_antisense_cuts, "
                                                  "       reverse_re_name, reverse_re_site, reverse_re_sense_cuts, reverse_re_antisense_cuts, "
                                                  "       forward_terminal_pattern, reverse_terminal_pattern, "
                                                  "       primer_tm_range_min, primer_tm_range_max, "
                                                  "       na_molar_concentration, primer_dna_molar_concentration, "
                                                  "       max_primer_pair_delta_tm "
                                                  "FROM primer_search_parameters "
                                                  "WHERE id = ? AND "
                                                  "      amplicon_length_min > 0 AND amplicon_length_min <= amplicon_length_max AND "
                                                  "      primer_length_min > 0 AND primer_length_min <= primer_length_max AND "
                                                  "      primer_tm_range_min <= primer_tm_range_max AND "
                                                  "      na_molar_concentration > 0 AND "
                                                  "      primer_dna_molar_concentration > 0 AND "
                                                  "      max_primer_pair_delta_tm >= 0");

    query.bindValue(0, primerSearchParametersId);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.next())
        return QSharedPointer<PrimerSearchParameters>();

    QSharedPointer<PrimerSearchParameters> psp(new PrimerSearchParameters(query.value(0).toInt()));
    psp->ampliconLengthRange_.begin_ = query.value(1).toInt();
    psp->ampliconLengthRange_.end_ = query.value(2).toInt();
    psp->primerLengthRange_.begin_ = query.value(3).toInt();
    psp->primerLengthRange_.end_ = query.value(4).toInt();

    QString REName = query.value(5).toString();
    BioString RESite = BioString(query.value(6).toByteArray(), eDnaGrammar);
    QVector<int> forwardCuts = RestrictionEnzyme::deserializeCutString(query.value(7).toString());
    QVector<int> reverseCuts = RestrictionEnzyme::deserializeCutString(query.value(8).toString());
    psp->forwardRestrictionEnzyme_ = RestrictionEnzyme(REName, RESite, forwardCuts, reverseCuts);

    REName = query.value(9).toString();
    RESite = BioString(query.value(10).toByteArray(), eDnaGrammar);
    forwardCuts = RestrictionEnzyme::deserializeCutString(query.value(11).toString());
    reverseCuts = RestrictionEnzyme::deserializeCutString(query.value(12).toString());
    psp->reverseRestrictionEnzyme_ = RestrictionEnzyme(REName, RESite, forwardCuts, reverseCuts);

    psp->forwardTerminalPattern_ = DnaPattern(query.value(13).toByteArray());
    psp->reverseTerminalPattern_ = DnaPattern(query.value(14).toByteArray());

    psp->individualPrimerTmRange_.begin_ = query.value(15).toDouble();
    psp->individualPrimerTmRange_.end_ = query.value(16).toDouble();

    psp->sodiumConcentration_ = query.value(17).toDouble();
    psp->primerDnaConcentration_ = query.value(18).toDouble();
    psp->maximumPrimerPairDeltaTm_ = query.value(19).toDouble();

    query.finish();

    return psp;
}

void DbPrimerSearchParametersCache::insertIntoCache(const QSharedPointer<PrimerSearchParameters> &primerSearchParameters)
{
    if (primerSearchParameters.isNull())
        return;

    cachedRecords_.insert(primerSearchParameters->id_, primerSearchParameters);
}
