/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <boost/make_shared.hpp>

#include <QtCore/QScopedPointer>

#include "PodEntityService.h"

#include "../Adoc.h"
#include "../ObservableMsa.h"
#include "../Entities/AminoMsa.h"
#include "../Entities/AminoSeq.h"
#include "../Entities/DnaMsa.h"
#include "../Entities/DnaSeq.h"
#include "../PODs/SimpleSeqPod.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adoc [Adoc *]
  */
PodEntityService::PodEntityService(Adoc *adoc) : adoc_(adoc)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param simpleSeqPods [const QVector<SimpleSeqPod> &]
  * @param isAlignment [bool]
  * @param grammar [Grammar]
  * @returns QVector<IEntitySPtr &>
  */
QVector<IEntitySPtr> PodEntityService::convertToEntities(const QVector<SimpleSeqPod> &simpleSeqPods, bool isAlignment, Grammar grammar)
{
    ASSERT(adoc_ != nullptr);

    if (isAlignment)
    {
        switch(grammar)
        {
        case eAminoGrammar:
            return QVector<IEntitySPtr>() << convertToAminoMsaEntity(simpleSeqPods);
        case eDnaGrammar:
            return QVector<IEntitySPtr>() << convertToDnaMsaEntity(simpleSeqPods);
        case eRnaGrammar:
            ASSERT_X(0, "Not implemented!");
            break;

        default:
            break;
        }
    }
    else
    {
        switch(grammar)
        {
        case eAminoGrammar:
            return convertToAminoSeqEntities(simpleSeqPods);
        case eDnaGrammar:
            return convertToDnaSeqEntities(simpleSeqPods);
            break;
        case eRnaGrammar:
            ASSERT_X(0, "Not implemented!");
            break;

        default:
            break;
        }
    }

    return QVector<IEntitySPtr>();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param simpleSeqPods [const QVector<SimpleSeqPod> &]
  * @returns QVector<IEntitySPtr>
  */
QVector<IEntitySPtr> PodEntityService::convertToAminoSeqEntities(const QVector<SimpleSeqPod> &simpleSeqPods) const
{
    QVector<IEntitySPtr> entities;
    entities.reserve(simpleSeqPods.size());

    AnonSeqRepository<Astring> *astringRepository = adoc_->astringRepository();
    ASSERT(astringRepository != nullptr);
    foreach (const SimpleSeqPod &pod, simpleSeqPods)
    {
        const AstringSPtr &astring = astringRepository->findBySeqOrCreate(pod.sequence_);
        ASSERT(astring);
        entities << AminoSeqSPtr(AminoSeq::createEntity(pod.name_, astring));
    }

    return entities;
}

/**
  * @param simpleSeqPods [const QVector<SimpleSeqPod> &]
  * @returns QVector<IEntitySPtr>
  */
QVector<IEntitySPtr> PodEntityService::convertToDnaSeqEntities(const QVector<SimpleSeqPod> &simpleSeqPods) const
{
    QVector<IEntitySPtr> entities;
    entities.reserve(simpleSeqPods.size());

    AnonSeqRepository<Dstring> *dstringRepository = adoc_->dstringRepository();
    ASSERT(dstringRepository != nullptr);
    foreach (const SimpleSeqPod &pod, simpleSeqPods)
    {
        const DstringSPtr &dstring = dstringRepository->findBySeqOrCreate(pod.sequence_);
        ASSERT(dstring);
        entities << IEntitySPtr(DnaSeq::createEntity(pod.name_, dstring));
    }

    return entities;
}

/**
  * @param simpleSeqPods [const QVector<SimpleSeqPod> &]
  * @returns QVector<IEntitySPtr &>
  */
QVector<IEntitySPtr> PodEntityService::convertToRnaSeqEntities(const QVector<SimpleSeqPod> &simpleSeqPods) const
{
    Q_UNUSED(simpleSeqPods);

    return QVector<IEntitySPtr>();
}

/**
  * @param simpleSeqPods [const QVector<SimpleSeqPod> &]
  * @returns IEntitySPtr
  */
IEntitySPtr PodEntityService::convertToAminoMsaEntity(const QVector<SimpleSeqPod> &simpleSeqPods) const
{
    QScopedPointer<ObservableMsa> msa(new ObservableMsa(eAminoGrammar));

    AnonSeqRepository<Astring> *astringRepository = adoc_->astringRepository();

    foreach (const SimpleSeqPod &pod, simpleSeqPods)
    {
        const AstringSPtr &astring = astringRepository->findBySeqOrCreate(pod.sequence_.ungapped());
        ASSERT(astring);
        Subseq *subseq = new Subseq(astring->seq_);
#ifdef QT_DEBUG
        ASSERT(subseq->setBioString(pod.sequence_));
#else
        subseq->setBioString(pod.sequence_);
#endif
        subseq->seqEntity_ = AminoSeqSPtr(AminoSeq::createEntity(pod.name_, astring));
#ifdef QT_DEBUG
        ASSERT(msa->append(subseq));
#else
        msa->append(subseq);
#endif
    }

    AminoMsaSPtr aminoMsa(AminoMsa::createEntity("New alignmment"));
    aminoMsa->setMsa(msa.take());

    return aminoMsa;
}

/**
  * @param simpleSeqPods [const QVector<SimpleSeqPod> &]
  * @returns IEntitySPtr
  */
IEntitySPtr PodEntityService::convertToDnaMsaEntity(const QVector<SimpleSeqPod> &simpleSeqPods) const
{
    QScopedPointer<ObservableMsa> msa(new ObservableMsa(eDnaGrammar));

    AnonSeqRepository<Dstring> *dstringRepository = adoc_->dstringRepository();

    foreach (const SimpleSeqPod &pod, simpleSeqPods)
    {
        const DstringSPtr &dstring = dstringRepository->findBySeqOrCreate(pod.sequence_.ungapped());
        ASSERT(dstring);
        Subseq *subseq = new Subseq(dstring->seq_);
#ifdef QT_DEBUG
        ASSERT(subseq->setBioString(pod.sequence_));
#else
        subseq->setBioString(pod.sequence_);
#endif
        subseq->seqEntity_ = DnaSeqSPtr(DnaSeq::createEntity(pod.name_, dstring));
#ifdef QT_DEBUG
        ASSERT(msa->append(subseq));
#else
        msa->append(subseq);
#endif
    }

    DnaMsaSPtr dnaMsa(DnaMsa::createEntity("New alignmment"));
    dnaMsa->setMsa(msa.take());

    return dnaMsa;
}
