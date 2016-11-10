/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DnaSeqFactory.h"
#include "../Entities/DnaSeq.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
DnaSeqFactory::DnaSeqFactory(AnonSeqRepository<Dstring> *dstringRepository, QObject *parent)
    : AbstractSeqFactory(parent),
      dstringRepository_(dstringRepository)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
AbstractSeqSPtr DnaSeqFactory::makeSeq(const QString &name, const QString &source, const BioString &dnaString) const
{
    if (dstringRepository_ == nullptr)
        return DnaSeqSPtr();

    const DstringSPtr dstring = dstringRepository_->findBySeqOrCreate(dnaString);
    DnaSeqSPtr dnaSeq = DnaSeqSPtr(DnaSeq::createEntity(name, dstring));
    dnaSeq->setSource(source);
    return dnaSeq;
}
