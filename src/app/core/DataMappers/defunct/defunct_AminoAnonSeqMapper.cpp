/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AminoAnonSeqMapper.h"
#include "../DataSources/IAdocSource.h"
#include "../Entities/AminoAnonSeq.h"
#include "../PODs/AnonSeqPod.h"
#include "../enums.h"
#include "../global.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocSource [IAdocSource *]
  */
AminoAnonSeqMapper::AminoAnonSeqMapper(IAdocSource *adocSource) : AbstractDataMapper<AminoAnonSeq>(adocSource)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param ids [const QVector<int> &]
  * @returns QVector<AminoAnonSeq *>
  */
QVector<AminoAnonSeq *> AminoAnonSeqMapper::find(const QVector<int> &ids) const
{
    try
    {
        QVector<AnonSeqPod> rootPods = adocSource_->readAminoAnonSeqs(ids);
        return reconstituteFromPods(rootPods);
    }
    // TODO: Fixme!
    catch(...)
    {
        return QVector<AminoAnonSeq *>();
    }
}

/**
  * @param digest [const QByteArray &]
  * @returns AminoAnonSeq *
  */
AminoAnonSeq *AminoAnonSeqMapper::findOneByDigest(const QByteArray &digest) const
{
    return findByDigests(QVector<QByteArray>() << digest).first();
}

/**
  * @param digests [const QByteArray &]
  * @returns QVector<AminoAnonSeq *>
  */
QVector<AminoAnonSeq *> AminoAnonSeqMapper::findByDigests(const QVector<QByteArray> &digests) const
{
    try
    {
        QVector<AnonSeqPod> rootPods = adocSource_->readAminoAnonSeqsFromDigests(digests);
        return reconstituteFromPods(rootPods);
    }
    catch(...)
    {
        return QVector<AminoAnonSeq *>();
    }
}

/**
  * @param aminoAnonSeqs [const QVector<AminoAnonSeq *> &]
  * @returns bool
  */
bool AminoAnonSeqMapper::save(const QVector<AminoAnonSeq *> &aminoAnonSeqs) const
{
    try
    {
        adocSource_->begin();
        foreach (AminoAnonSeq *aminoAnonSeq, aminoAnonSeqs)
        {
            ASSERT(aminoAnonSeq);
            ASSERT(aminoAnonSeq->id() != 0);

            if (aminoAnonSeq->isNew())
            {
                AnonSeqPod pod(aminoAnonSeq->id(), aminoAnonSeq->seq_.asByteArray(), aminoAnonSeq->seq_.digest());
                adocSource_->insertAminoAnonSeq(pod);
                aminoAnonSeq->setId(pod.id_);
            }

            // Update the coil data
            QVector<CoilPod> coilPods = coilMapper_.mapFromObjects(aminoAnonSeq->coils());
            adocSource_->saveCoils(aminoAnonSeq->id(), coilPods);           // This synchronizes the database coils for this sequence
                                                                            // creating new records for new coils and associating datasource
                                                                            // generated ids with the pod data
            aminoAnonSeq->setCoils(coilMapper_.mapToObjects(coilPods));     // Update the coils

            // Update the seg data
            QVector<SegPod> segPods = segMapper_.mapFromObjects(aminoAnonSeq->segs_);
            adocSource_->saveSegs(aminoAnonSeq->id(), segPods);
            aminoAnonSeq->setSegs(segMapper_.mapToObjects(segPods));
        }
        adocSource_->end();
    }
    // TODO: FIXME!
    catch (...)
    {
        return false;
    }

    return true;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param aminoAnonSeqPods [const QVector<AnonSeqPod> &]
  * @returns QVector<AminoAnonSeq *>
  */
QVector<AminoAnonSeq *> AminoAnonSeqMapper::reconstituteFromPods(const QVector<AnonSeqPod> &aminoAnonSeqPods) const
{
    QVector<AminoAnonSeq *> aminoAnonSeqs;
    aminoAnonSeqs.reserve(aminoAnonSeqPods.size());
    foreach (const AnonSeqPod &aminoAnonSeqPod, aminoAnonSeqPods)
    {
        AminoAnonSeq *aminoAnonSeq = nullptr;

        // The id will be greater than zero if it is found in the database
        if (aminoAnonSeqPod.id_ > 0)
        {
            aminoAnonSeq = new AminoAnonSeq(aminoAnonSeqPod.id_, Seq(aminoAnonSeqPod.sequence_, eAminoGrammar));
            aminoAnonSeq->coils_ = coilMapper_.mapToObjects(adocSource_->readCoils(aminoAnonSeqPod.id_, aminoAnonSeq->seq_.length()));
            aminoAnonSeq->segs_ = segMapper_.mapToObjects(adocSource_->readSegs(aminoAnonSeqPod.id_, aminoAnonSeq->seq_.length()));
        }

        aminoAnonSeqs << aminoAnonSeq;
    }

    return aminoAnonSeqs;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param ids [const QVector<int> &]
  */
void AminoAnonSeqMapper::erase(const QVector<int> &ids) const
{
    adocSource_->eraseAminoAnonSeqs(ids);
}

