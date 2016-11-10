/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTREPORT_H
#define BLASTREPORT_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include "AbstractBasicEntity.h"
#include "../util/ClosedIntRange.h"
#include "../PODs/BlastDatabaseSpec.h"
#include "../PODs/HitPod.h"

struct BlastReportPod;

class BlastReport : public AbstractBasicEntity
{
public:
    typedef boost::shared_ptr<BlastReport> SPtr;
    static const int kType;

    // ------------------------------------------------------------------------------------------------
    // Constructors
    BlastReport(int id,
                int querySeqId,
                const ClosedIntRange &queryRange,
                const BioString &query,
                const QString &sourceFile,
                const QString &name,
                const QString &description,
                const QString &notes,
                const BlastDatabaseSpec &databaseSpec);
    BlastReport(const BlastReportPod &blastReportPod);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BlastDatabaseSpec databaseSpec() const;
    QVector<HitPod> hits() const;
    bool isLoaded() const;
    BioString query() const;
    ClosedIntRange queryRange() const;
    int querySeqId() const;
    void setHits(const QVector<HitPod> &hits);
    void setLoaded(bool loaded);
    QString sourceFile() const;
    int type() const;


    // ------------------------------------------------------------------------------------------------
    // Static methods
    static BlastReport *createEntity(int querySeqId,
                                     const ClosedIntRange &queryRange,
                                     const BioString &query,
                                     const QString &sourceFile,
                                     const QString &name,
                                     const QString &description,
                                     const QString &notes,
                                     const BlastDatabaseSpec &databaseSpec);

    static BlastReport *createEntity(int querySeqId,
                                     const ClosedIntRange &queryRange,
                                     const BioString &query,
                                     const QString &sourceFile,
                                     const QString &name,
                                     const BlastDatabaseSpec &databaseSpec);

private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    bool loaded_;
    int querySeqId_;            // Either amino_seq_id or dna_seq_id
    ClosedIntRange queryRange_; // The range of the corresponding querySeqId used as the query for the blast - should
                                // exactly match query_
    BioString query_;
    QString sourceFile_;

    BlastDatabaseSpec databaseSpec_;

    QVector<HitPod> hits_;
};

Q_DECLARE_TYPEINFO(BlastReport, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<BlastReport> BlastReportSPtr;

struct BlastReportPod : public AbstractBasicEntityPod
{
    int querySeqId_;
    ClosedIntRange queryRange_;
    BioString query_;
    QString sourceFile_;
    BlastDatabaseSpec databaseSpec_;

    BlastReportPod(int id = 0)
        : AbstractBasicEntityPod(id), querySeqId_(0)
    {
    }
};

#endif // BLASTREPORT_H
