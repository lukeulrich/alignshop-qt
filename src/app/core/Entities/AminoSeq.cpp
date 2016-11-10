/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AminoSeq.h"
#include "../macros.h"

const int AminoSeq::kType = eAminoSeqEntity;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param id [int]
  * @param start [int]
  * @param stop [int]
  * @param name [const QString &]
  * @param source [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  * @param astring [Astring *]
  */
AminoSeq::AminoSeq(int id,
                   int start,
                   int stop,
                   const QString &name,
                   const QString &source,
                   const QString &description,
                   const QString &notes,
                   const AstringSPtr &astring)
    : AbstractSeq(id, start, stop, name, source, description, notes, astring)
{
    ASSERT(astring);
}

/**
  * Specialized constructor intended to permit construction from corresponding AstringPod object. Only the Crud classes
  * should call this constructor.
  *
  * @param aminoSeqPod [const AminoSeqPod &]
  */
AminoSeq::AminoSeq(const AminoSeqPod &aminoSeqPod)
        : AbstractSeq(aminoSeqPod.id_,
                            aminoSeqPod.start_,
                            aminoSeqPod.stop_,
                            aminoSeqPod.name_,
                            aminoSeqPod.source_,
                            aminoSeqPod.description_,
                            aminoSeqPod.notes_,
                            aminoSeqPod.astring_)
{
}

/**
  * @returns AminoSeq *
  */
AminoSeq *AminoSeq::clone() const
{
    return new AminoSeq(::newEntityId<AminoSeq>(),
                        start_,
                        stop_,
                        name_,
                        source_,
                        description_,
                        notes_,
                        boost::shared_static_cast<Astring>(abstractAnonSeq_));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Astring *
  */
AstringSPtr AminoSeq::abstractAnonSeq() const
{
    return boost::shared_static_cast<Astring>(abstractAnonSeq_);
}

/**
  * @returns int
  */
int AminoSeq::type() const
{
    return eAminoSeqEntity;
}

/**
  * @param aminoSeqs [const QVector<AminoSeq *> &]
  * @returns QVector<int>
  */
QVector<int> AminoSeq::astringIdVector(const QVector<AminoSeq *> &aminoSeqs)
{
    QVector<int> astringIds;
    astringIds.reserve(aminoSeqs.size());
    foreach (const AminoSeq *aminoSeq, aminoSeqs)
        astringIds << aminoSeq->abstractAnonSeq()->id();

    return astringIds;
}

/**
  * Factory method for generating new AminoSeq Entities with start and stop spanning entire astring
  *
  * @param name [const QString &]
  * @param astring [Astring *]
  * @returns AminoSeq *
  */
AminoSeq *AminoSeq::createEntity(const QString &name, const AstringSPtr &astring)
{
    ASSERT(astring != nullptr);
    return new AminoSeq(::newEntityId<AminoSeq>(),
                        1,
                        astring->seq_.length(),
                        name,
                        QString(),
                        QString(),
                        QString(),
                        astring);
}

/**
  * Factory method for generating new AminoSeq Entities.
  *
  * @param start [int]
  * @param stop [int]
  * @param name [const QString &]
  * @param source [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  * @param astring [Astring *]
  * @returns AminoSeq *
  */
AminoSeq *AminoSeq::createEntity(int start,
                                 int stop,
                                 const QString &name,
                                 const QString &source,
                                 const QString &description,
                                 const QString &notes,
                                 const AstringSPtr &astring)
{
    ASSERT(astring != nullptr);
    return new AminoSeq(::newEntityId<AminoSeq>(),
                        start,
                        stop,
                        name,
                        source,
                        description,
                        notes,
                        astring);
}

