/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AminoSeqMapper.h"
#include "../Entities/AminoSeq.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocSource [IAdocSource *]
  * @param astringRepository [AnonSeqRepository<Astring> *]
  */
AminoSeqMapper::AminoSeqMapper(IAdocSource *adocSource, AnonSeqRepository<Astring> *astringRepository) :
    GenericEntityMapper<AminoSeq, AminoSeqPod>(adocSource), astringRepository_(astringRepository)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param aminoSeqs [const QVector<AminoSeq *> &]
  * @returns bool
  */
bool AminoSeqMapper::save(const QVector<AminoSeq *> &aminoSeqs) const
{
    try
    {
        astringRepository_->save(AminoSeq::astringIdVector(aminoSeqs));
        return GenericEntityMapper<AminoSeq, AminoSeqPod>::save(aminoSeqs);
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
  * Called by the virtual find method and is responsible for converting the raw pod data into entity objects. Pods
  * currently contains the raw data corresponding for several AminoSeqs; however, it is necessary to map the Astring
  * entities before they may be converted into objects.
  *
  * Note: pods cannot be a const reference because it is configured with the relevant astring.
  *
  * @param pods [QVector<AminoSeqPod> &]
  * @returns QVector<AminoSeq *>
  */
QVector<AminoSeq *> AminoSeqMapper::convertPodsToEntities(QVector<AminoSeqPod> &pods) const
{
    // Fetch the astrings
    QVector<int> astringIds;
    astringIds.reserve(pods.size());
    foreach (const AminoSeqPod &pod, pods)
        astringIds << pod.astringId_;

    const QVector<AstringSPtr> &astrings = astringRepository_->find(astringIds);
    ASSERT(astrings.size() == pods.size());

    QVector<AminoSeq *> aminoSeqs(pods.size(), nullptr);
    for (int i=0, z=pods.size(); i<z; ++i)
    {
        if (astrings.at(i) != nullptr)
        {
            pods[i].astring_ = astrings.at(i);
            aminoSeqs[i] = new AminoSeq(pods[i]);
        }
    }

    return aminoSeqs;
}
