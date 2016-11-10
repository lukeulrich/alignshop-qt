/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AminoAnonSeqRepository.h"
#include "../DataMappers/AminoAnonSeqMapper.h"
#include "../Seq.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param aminoAnonSeqMapper [AminoAnonSeqMapper *]
  */
AminoAnonSeqRepository::AminoAnonSeqRepository(AminoAnonSeqMapper *aminoAnonSeqMapper) :
    GenericRepository<AminoAnonSeq>(aminoAnonSeqMapper)
{
    ASSERT(aminoAnonSeqMapper);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param aminoAnonSeqs [const QVector<AminoAnonSeq *> &]
  * @returns bool
  */
bool AminoAnonSeqRepository::add(const QVector<AminoAnonSeq *> &aminoAnonSeqs, bool ignoreNullPointers)
{
    if (!GenericRepository<AminoAnonSeq>::add(aminoAnonSeqs, ignoreNullPointers))
        return false;

    foreach (AminoAnonSeq *aminoAnonSeq, aminoAnonSeqs)
    {
        if (ignoreNullPointers && aminoAnonSeq == nullptr)
            continue;

        ASSERT(seqIdentityMap_.contains(aminoAnonSeq->seq_.digest()) == false);
        seqIdentityMap_.insert(aminoAnonSeq->seq_.digest(), aminoAnonSeq);
    }

    return true;
}

/**
  * @param object [AminoAnonSeq *]
  */
/*
void AminoAnonSeqRepository::remove(const QVector<AminoAnonSeq *> &aminoAnonSeqs)
{
    foreach (const AminoAnonSeq *aminoAnonSeq, aminoAnonSeqs)
    {
        ASSERT(aminoAnonSeq);
        ASSERT(seqIdentityMap_.contains(aminoAnonSeq->seq_.digest()));
        seqIdentityMap_.remove(aminoAnonSeq->seq_.digest());
    }

    GenericRepository<AminoAnonSeq>::remove(aminoAnonSeqs);
}
*/

/**
  * @param seq [const Seq &]
  * @returns AminoAnonSeq *
  */
AminoAnonSeq *AminoAnonSeqRepository::findBySeq(const Seq &seq)
{
    ASSERT(seq.grammar() == eAminoGrammar);

    QByteArray digest = seq.digest();
    if (seqIdentityMap_.contains(digest))
        return seqIdentityMap_.value(digest);

    AminoAnonSeq *aminoAnonSeq = static_cast<AminoAnonSeqMapper *>(dataMapper_)->findOneByDigest(digest);
    addOne(aminoAnonSeq, false);  // Ignore null pointers
    return aminoAnonSeq;
}

/**
  * @param seq [const Seq &]
  * @returns AminoAnonSeq *
  */
AminoAnonSeq *AminoAnonSeqRepository::findBySeqOrCreate(const Seq &seq)
{
    ASSERT(seq.grammar() == eAminoGrammar);

    AminoAnonSeq *aminoAnonSeq = findBySeq(seq);
    if (aminoAnonSeq)
        return aminoAnonSeq;

    // No object found for this sequence, create a new record
    aminoAnonSeq = AminoAnonSeq::create(seq);
    addOne(aminoAnonSeq, false);  // Ignore null pointers
    return aminoAnonSeq;
}

/**
  * @param aminoAnonSeqs [const QVector<AminoAnonSeq *>]
  * @returns bool
  */
bool AminoAnonSeqRepository::erase(const QVector<AminoAnonSeq *> &aminoAnonSeqs)
{
    if (!GenericRepository<AminoAnonSeq>::erase(aminoAnonSeqs))
        return false;

    // Remove digests from seqIdentity Hash
    foreach (const AminoAnonSeq *aminoAnonSeq, aminoAnonSeqs)
    {
        ASSERT(aminoAnonSeq);
        ASSERT(seqIdentityMap_.contains(aminoAnonSeq->seq_.digest()));
        seqIdentityMap_.remove(aminoAnonSeq->seq_.digest());
    }

    return true;
}
