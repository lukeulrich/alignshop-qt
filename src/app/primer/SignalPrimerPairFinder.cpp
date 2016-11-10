/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SignalPrimerPairFinder.h"
#include "PrimerPairFinder.h"
#include "PrimerSearchParameters.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
SignalPrimerPairFinder::SignalPrimerPairFinder(QObject *parent)
    : QObject(parent),
      primerPairFinder_(nullptr)
{
}

/**
  */
SignalPrimerPairFinder::~SignalPrimerPairFinder()
{
    delete primerPairFinder_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void SignalPrimerPairFinder::cancel()
{
    if (primerPairFinder_ == nullptr)
        return;

    primerPairFinder_->cancel();
}

/**
  * @param input [const PrimerSearchParameters &]
  */
void SignalPrimerPairFinder::findPrimerPairs(const BioString &dnaString, const ClosedIntRange &range, const PrimerSearchParameters &input)
{
    if (primerPairFinder_ == nullptr)
        primerPairFinder_ = new PrimerPairFinder;

    QVector<PrimerPair> results = primerPairFinder_->findPrimerPairs(dnaString, range, input);

    emit finished(results);
}
