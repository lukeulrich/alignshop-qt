/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERPAIRFACTORY_H
#define PRIMERPAIRFACTORY_H

class QString;

class Primer;
class PrimerPair;

/**
  * PrimerPairFactory encapsulates creating PrimerPair properly initialized objects and in particular deriving their
  * score.
  */
class PrimerPairFactory
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    PrimerPair makePrimerPair(const Primer &forwardPrimer, const Primer &reversePrimer);
    PrimerPair makePrimerPair(const QString &name, const Primer &forwardPrimer, const Primer &reversePrimer);
};

#endif // PRIMERPAIRFACTORY_H
