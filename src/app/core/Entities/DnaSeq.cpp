/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DnaSeq.h"
#include "EntityFlags.h"
#include "../macros.h"

const int DnaSeq::kType = eDnaSeqEntity;

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
  * @param dstring [Dstring *]
  */
DnaSeq::DnaSeq(int id,
               int start,
               int stop,
               const QString &name,
               const QString &source,
               const QString &description,
               const QString &notes,
               const DstringSPtr &dstring)
    : AbstractSeq(id, start, stop, name, source, description, notes, dstring)
{
    ASSERT(dstring);
}

/**
  * Specialized constructor intended to permit construction from corresponding AstringPod object. Only the Crud classes
  * should call this constructor.
  *
  * @param dnaSeqPod [const DnaSeqPod &]
  */
DnaSeq::DnaSeq(const DnaSeqPod &dnaSeqPod)
        : AbstractSeq(dnaSeqPod.id_,
                      dnaSeqPod.start_,
                      dnaSeqPod.stop_,
                      dnaSeqPod.name_,
                      dnaSeqPod.source_,
                      dnaSeqPod.description_,
                      dnaSeqPod.notes_,
                      dnaSeqPod.dstring_)
{
    primers_ = dnaSeqPod.primers_;
}

/**
  * @returns DnaSeq *
  */
DnaSeq *DnaSeq::clone() const
{
    return new DnaSeq(::newEntityId<DnaSeq>(),
                      start_,
                      stop_,
                      name_,
                      source_,
                      description_,
                      notes_,
                      boost::shared_static_cast<Dstring>(abstractAnonSeq_));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Dstring *
  */
DstringSPtr DnaSeq::abstractAnonSeq() const
{
    return boost::shared_static_cast<Dstring>(abstractAnonSeq_);
}

/**
  * @returns int
  */
int DnaSeq::type() const
{
    return eDnaSeqEntity;
}

//int DnaSeq::primerPairCount() const
//{
//    return primerPairs_.size();
//}

//void DnaSeq::appendPrimerPairs(const PrimerPairVector &newPrimerPairs)
//{
//    if (newPrimerPairs.size() == 0)
//        return;

//    primerPairs_ << newPrimerPairs;
//    setDirty(Ag::ePrimerPairsFlag, true);
//}

//PrimerPairVector DnaSeq::primerPairs() const
//{
//    return primerPairs_;
//}

//void DnaSeq::removePrimerPairAt(const int primerPairIndex)
//{
//    primerPairs_.remove(primerPairIndex, 1);
//    setDirty(Ag::ePrimerPairsFlag, true);
//}

//void DnaSeq::setPrimerPairs(const PrimerPairVector &newPrimerPairs)
//{
//    primerPairs_ = newPrimerPairs;
//    setDirty(Ag::ePrimerPairsFlag, true);
//}


// ---------------
// Helper routines
QVector<int> DnaSeq::dstringIdVector(const QVector<DnaSeq *> &dnaSeqs)
{
    QVector<int> dstringIds;
    dstringIds.reserve(dnaSeqs.size());
    foreach (const DnaSeq *dnaSeq, dnaSeqs)
        dstringIds << dnaSeq->abstractAnonSeq()->id();

    return dstringIds;
}

/**
  * Factory method for generating new DnaSeq Entities with start and stop spanning entire dstring
  *
  * @param name [const QString &]
  * @param dstring [Dstring *]
  * @returns DnaSeq *
  */
DnaSeq *DnaSeq::createEntity(const QString &name, const DstringSPtr &dstring)
{
    ASSERT(dstring != nullptr);
    return new DnaSeq(::newEntityId<DnaSeq>(),
                      1,
                      dstring->seq_.length(),
                      name,
                      QString(),
                      QString(),
                      QString(),
                      dstring);
}

/**
  * Factory method for generating new DnaSeq Entities.
  *
  * @param start [int]
  * @param stop [int]
  * @param name [const QString &]
  * @param source [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  * @param dstring [Dstring *]
  * @returns DnaSeq *
  */
DnaSeq *DnaSeq::createEntity(int start,
                             int stop,
                             const QString &name,
                             const QString &source,
                             const QString &description,
                             const QString &notes,
                             const DstringSPtr &dstring)
{
    ASSERT(dstring != nullptr);
    return new DnaSeq(::newEntityId<DnaSeq>(),
                      start,
                      stop,
                      name,
                      source,
                      description,
                      notes,
                      dstring);
}

