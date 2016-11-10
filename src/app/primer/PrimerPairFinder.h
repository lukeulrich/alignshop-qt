/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERPAIRFINDER_H
#define PRIMERPAIRFINDER_H

#include <QtCore/QVector>

#include "PrimerPair.h"
#include "PrimerSearchParameters.h"
#include "../core/util/ClosedIntRange.h"
#include "../core/util/Range.h"

class BioString;
class DnaPattern;

/**
  * Only works on stretches of DNA sequence comprised of ACGT.
  */
class PrimerPairFinder
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    PrimerPairFinder();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void cancel();
    QVector<PrimerPair> findPrimerPairs(const BioString &dnaString, const ClosedIntRange &range, const PrimerSearchParameters &primerSearchParameters);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    struct LitePrimer;

    int absoluteMaxPrimerStart(const ClosedIntRange &range) const;
    bool rangeIsLessThanMinimumPrimerLength(const ClosedIntRange &range) const;
    QVector<ClosedIntRange> findACGTRangesWithin(const BioString &dnaString, const ClosedIntRange &range) const;
    QVector<LitePrimer> findCompatibleLitePrimers(const BioString &dnaString,
                                                  const ClosedIntRange &range,
                                                  const int absoluteMaxPrimerStart,
                                                  const RestrictionEnzyme &restrictionEnzyme,
                                                  const DnaPattern &terminalPattern);
    QVector<PrimerPair> findCompatiblePrimerPairs(const QVector<LitePrimer> &forwardPrimers,
                                                  const QVector<LitePrimer> &reversePrimers,
                                                  const BioString &dnaString);
    bool isACGT(const char nucleotide) const;


    // ------------------------------------------------------------------------------------------------
    // Private structures
    /**
      * LitePrimer is a lightweight container of the melting temperature for a primer located at a specific
      * location within a target sequence.
      *
      * It is important to note that the tm_ may or may not refer to a primer that has an associated restriction enzyme.
      */
    struct LitePrimer
    {
        double tm_;                 //!< Melting temperature in degrees Celsius
        ClosedIntRange location_;   //!< Range on source dnaString

        LitePrimer()
            : tm_(0)
        {
        }

        LitePrimer(const double tm, const ClosedIntRange &location)
            : tm_(tm),
              location_(location)
        {
        }
    };


    // ------------------------------------------------------------------------------------------------
    // Private members
    bool canceled_;
    PrimerSearchParameters primerSearchParameters_;
};

#endif // PRIMERPAIRFINDER_H
