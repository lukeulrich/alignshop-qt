/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DnaSeqMapper.h"
#include "../Entities/DnaSeq.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocSource [IAdocSource *]
  * @param dstringRepository [AnonSeqRepository<Dstring> *]
  */
DnaSeqMapper::DnaSeqMapper(IAdocSource *adocSource, AnonSeqRepository<Dstring> *dstringRepository) :
    GenericEntityMapper<DnaSeq, DnaSeqPod>(adocSource), dstringRepository_(dstringRepository)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param dnaSeqs [const QVector<DnaSeq *> &]
  * @returns bool
  */
bool DnaSeqMapper::save(const QVector<DnaSeq *> &dnaSeqs) const
{
    try
    {
        dstringRepository_->save(DnaSeq::dstringIdVector(dnaSeqs));
        return GenericEntityMapper<DnaSeq, DnaSeqPod>::save(dnaSeqs);
    }
    catch(...)
    {
        return false;
    }
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param pods [QVector<AminoSeqPod> &]
  * @returns QVector<AminoSeq *>
  */
QVector<DnaSeq *> DnaSeqMapper::convertPodsToEntities(QVector<DnaSeqPod> &pods) const
{
    // Fetch the astrings
    QVector<int> dstringIds;
    dstringIds.reserve(pods.size());
    foreach (const DnaSeqPod &pod, pods)
        dstringIds << pod.dstringId_;

    const QVector<DstringSPtr> &dstrings = dstringRepository_->find(dstringIds);
    ASSERT(dstrings.size() == pods.size());

    QVector<DnaSeq *> dnaSeqs(pods.size(), nullptr);
    for (int i=0, z=pods.size(); i<z; ++i)
    {
        if (dstrings.at(i) != nullptr)
        {
            pods[i].dstring_ = dstrings.at(i);
            dnaSeqs[i] = new DnaSeq(pods[i]);
        }
    }

    return dnaSeqs;
}
