/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MAXCOVERAGEALPHABETDETECTOR_H
#define MAXCOVERAGEALPHABETDETECTOR_H

#include "AbstractAlphabetDetector.h"

/**
  * Determines the alphabet of a BioString using a maximum alphabet coverage strategy.
  *
  * Each AlphabetInspector contains an associated BioStringValidator which in turn has a fixed
  * number of validation characters. This is the maximum alphabet coverage a BioString can have.
  * MaxCoverageAlphabetDetector exploits this property to determine a compatible Alphabet with
  * the highest coverage. For example,
  *
  * Given the sequence: ACT
  * Covers 3/20 of the amino acid alphabet and thus its coverage is: 15%
  * Covers 3/4 of the DNA alphabet, coverage = 75%
  * Result --> DnaAlphabet because it has a higher coverage (and is more specific)
  *
  * This class overrides the setInspectors function such that they can be sorted according to their
  * alphabetic "specificities".
  */
class MaxCoverageAlphabetDetector : public AbstractAlphabetDetector
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    Alphabet detectAlphabet(const BioString &bioString) const;  //!< Detect and return the alphabet of bioString based on the maximum covering alphabet
    //! Sets the list of alphabet inspectors to inspectors for analyzing BioString(s)
    virtual void setInspectors(const QList<AlphabetInspector> &inspectors);
};

#endif // MAXCOVERAGEALPHABETDETECTOR_H
