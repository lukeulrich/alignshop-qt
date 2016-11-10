/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>

#include "BlastReport.h"
#include "../BioString.h"
#include "../global.h"
#include "../macros.h"

const int BlastReport::kType = eBlastReportEntity;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param id [int]
  * @param querySeqId [int]
  * @param queryRange [const ClosedIntRange &]
  * @param query [const BioString &]
  * @param sourceFile [const QString &]
  * @param name [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  * @param databaseSpec [const BlastDatabaseSpec &]
  */
BlastReport::BlastReport(int id,
                         int querySeqId,
                         const ClosedIntRange &queryRange,
                         const BioString &query,
                         const QString &sourceFile,
                         const QString &name,
                         const QString &description,
                         const QString &notes,
                         const BlastDatabaseSpec &databaseSpec)
    : AbstractBasicEntity(id, name, description, notes),
      loaded_(false),
      querySeqId_(querySeqId),
      queryRange_(queryRange),
      query_(query),
      sourceFile_(sourceFile),
      databaseSpec_(databaseSpec)
{
    ASSERT(query_.length() == queryRange_.length());
    ASSERT(query_.grammar() == eAminoGrammar || query_.grammar() == eDnaGrammar);
}

/**
  * @param blastReportPod [const BlastReportPod &]
  */
BlastReport::BlastReport(const BlastReportPod &blastReportPod)
    : AbstractBasicEntity(blastReportPod.id_,
                          blastReportPod.name_,
                          blastReportPod.description_,
                          blastReportPod.notes_),
      loaded_(false),
      querySeqId_(blastReportPod.querySeqId_),
      queryRange_(blastReportPod.queryRange_),
      query_(blastReportPod.query_),
      sourceFile_(blastReportPod.sourceFile_),
      databaseSpec_(blastReportPod.databaseSpec_)
{
    ASSERT(query_.length() == queryRange_.length());
    ASSERT(query_.grammar() == eAminoGrammar || query_.grammar() == eDnaGrammar);
}

/**
  * @returns BlastDatabaseSpec
  */
BlastDatabaseSpec BlastReport::databaseSpec() const
{
    return databaseSpec_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QVector<HitPod>
  */
QVector<HitPod> BlastReport::hits() const
{
    return hits_;
}

/**
  * @returns bool
  */
bool BlastReport::isLoaded() const
{
    return loaded_;
}

/**
  * @returns BioString
  */
BioString BlastReport::query() const
{
    return query_;
}

/**
  * @returns ClosedIntRange
  */
ClosedIntRange BlastReport::queryRange() const
{
    return queryRange_;
}

/**
  * @returns int
  */
int BlastReport::querySeqId() const
{
    return querySeqId_;
}

/**
  * @param hits [const QVector<HitPod> &]
  */
void BlastReport::setHits(const QVector<HitPod> &hits)
{
    hits_ = hits;
}

/**
  * @param loaded [bool]
  */
void BlastReport::setLoaded(bool loaded)
{
    loaded_ = loaded;
}

/**
  * @returns QString
  */
QString BlastReport::sourceFile() const
{
    return sourceFile_;
}

/**
  * @returns int
  */
int BlastReport::type() const
{
    return eBlastReportEntity;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public static methods
/**
  * @param querySeqId [int]
  * @param queryRange [const ClosedIntRange &]
  * @param query [const BioString &]
  * @param sourceFile [const QString &]
  * @param name [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  * @param databaseSpec [const BlastDatabaseSpec &]
  * @returns BlastReport *
  */
BlastReport *BlastReport::createEntity(int querySeqId,
                                       const ClosedIntRange &queryRange,
                                       const BioString &query,
                                       const QString &sourceFile,
                                       const QString &name,
                                       const QString &description,
                                       const QString &notes,
                                       const BlastDatabaseSpec &databaseSpec)
{
    return new BlastReport(::newEntityId<BlastReport>(),
                           querySeqId,
                           queryRange,
                           query,
                           sourceFile,
                           name,
                           description,
                           notes,
                           databaseSpec);
}

/**
  * @param querySeqId [int]
  * @param queryRange [const ClosedIntRange &]
  * @param query [const BioString &]
  * @param sourceFile [const QString &]
  * @param name [const QString &]
  * @param databaseSpec [const BlastDatabaseSpec &]
  * @returns BlastReport *
  */
BlastReport *BlastReport::createEntity(int querySeqId,
                                       const ClosedIntRange &queryRange,
                                       const BioString &query,
                                       const QString &sourceFile,
                                       const QString &name,
                                       const BlastDatabaseSpec &databaseSpec)
{
    return new BlastReport(::newEntityId<BlastReport>(),
                           querySeqId,
                           queryRange,
                           query,
                           sourceFile,
                           name,
                           "",
                           "",
                           databaseSpec);
}
