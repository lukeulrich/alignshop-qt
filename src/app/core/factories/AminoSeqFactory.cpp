/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AminoSeqFactory.h"
#include "../Entities/AminoSeq.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
AminoSeqFactory::AminoSeqFactory(AnonSeqRepository<Astring> *astringRepository, QObject *parent)
    : AbstractSeqFactory(parent),
      astringRepository_(astringRepository)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
AbstractSeqSPtr AminoSeqFactory::makeSeq(const QString &name, const QString &source, const BioString &aminoString) const
{
    if (astringRepository_ == nullptr)
        return AminoSeqSPtr();

    const AstringSPtr astring = astringRepository_->findBySeqOrCreate(aminoString);
    AminoSeqSPtr aminoSeq = AminoSeqSPtr(AminoSeq::createEntity(name, astring));
    aminoSeq->setSource(source);
    return aminoSeq;
}
