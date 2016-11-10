/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "PrimerPairFactory.h"
#include "Primer.h"
#include "PrimerPair.h"
#include "DimerScoreCalculator.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
PrimerPair PrimerPairFactory::makePrimerPair(const Primer &forwardPrimer, const Primer &reversePrimer)
{
    return makePrimerPair(QString(), forwardPrimer, reversePrimer);
}

PrimerPair PrimerPairFactory::makePrimerPair(const QString &name, const Primer &forwardPrimer, const Primer &reversePrimer)
{
    DimerScoreCalculator dimerScoreCalculator;
    double heteroDimerScore = dimerScoreCalculator.dimerScore(forwardPrimer.sequence(), reversePrimer.sequence());
    double finalScore = PrimerPair::deltaTm(forwardPrimer, reversePrimer) +
                        forwardPrimer.homoDimerScore() +
                        reversePrimer.homoDimerScore() +
                        heteroDimerScore;
    PrimerPair newPrimerPair = PrimerPair(forwardPrimer, reversePrimer, finalScore);
    newPrimerPair.setName(name);
    return newPrimerPair;
}
