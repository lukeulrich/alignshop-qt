/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "PrimerItemFactory.h"
#include "../../core/macros.h"
#include "../../primer/Primer.h"
#include "../painting/features/ForwardPrimerItem.h"
#include "../painting/features/ReversePrimerItem.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
PrimerItemFactory::PrimerItemFactory(QObject *parent)
    : QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void PrimerItemFactory::setDnaSeq(const DnaSeqSPtr &dnaSeq)
{
    dnaSeq_ = dnaSeq;
}

QVector<AbstractPrimerItem *> PrimerItemFactory::makePrimerItemsFromPrimer(const Primer &primer) const
{
    if (!dnaSeq_)
        return QVector<AbstractPrimerItem *>();

    QVector<AbstractPrimerItem *> primerItems;
    primerItems << makeForwardPrimerItemsFromPrimer(primer);
    primerItems << makeReversePrimerItemsFromPrimer(primer);
    return primerItems;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
QVector<AbstractPrimerItem *> PrimerItemFactory::makeForwardPrimerItemsFromPrimer(const Primer &primer) const
{
    ASSERT(dnaSeq_ != nullptr);

    QVector<AbstractPrimerItem *> forwardPrimerItems;
    QVector<ClosedIntRange> forwardLocations = primer.coreSequenceForwardLocationsIn(dnaSeq_->parentBioString());
    foreach (const ClosedIntRange &location, forwardLocations)
        forwardPrimerItems << new ForwardPrimerItem(primer.id(), primer.name(), location);
    return forwardPrimerItems;
}

QVector<AbstractPrimerItem *> PrimerItemFactory::makeReversePrimerItemsFromPrimer(const Primer &primer) const
{
    ASSERT(dnaSeq_ != nullptr);

    QVector<AbstractPrimerItem *> reversePrimerItems;
    QVector<ClosedIntRange> reverseLocations = primer.coreSequenceReverseLocationsIn(dnaSeq_->parentBioString());
    foreach (const ClosedIntRange &location, reverseLocations)
        reversePrimerItems << new ReversePrimerItem(primer.id(), primer.name(), location);
    return reversePrimerItems;
}
