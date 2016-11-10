/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DynamicSeqFactory.h"
#include "AbstractSeqFactory.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
DynamicSeqFactory::DynamicSeqFactory(QObject *parent)
    : QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
AbstractSeqSPtr DynamicSeqFactory::makeSeq(const QString &name, const QString &source, const BioString &bioString) const
{
    if (!grammarSeqFactoryHash_.contains(bioString.grammar()))
        return AbstractSeqSPtr();

    AbstractSeqFactory *factory = grammarSeqFactoryHash_.value(bioString.grammar());
    return factory->makeSeq(name, source, bioString);
}

void DynamicSeqFactory::setSeqFactoryForGrammar(const Grammar grammar, AbstractSeqFactory *seqFactory)
{
    if (seqFactory == nullptr)
        return;

    grammarSeqFactoryHash_.insert(grammar, seqFactory);
}
