/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "MaxCoverageAlphabetDetector.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Forward declarations
bool validatorCoverageLessThan(const AlphabetInspector &a, const AlphabetInspector &b);


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Iterates through the sorted list of inspectors and returns the alphabet from the first inspector
  * that returns a known alphabet. Returns eUnknownAlphabet if there are no inspectors defined or
  * they all returned eUnknownAlphabet.
  *
  * @param bioString [const BioString &]
  * @returns Alphabet
  */
Alphabet MaxCoverageAlphabetDetector::detectAlphabet(const BioString &bioString) const
{
    if (inspectors_.size() > 0)
    {
        // Remember: inspectors_ are sorted based on their character specificity (valid character size of associated validators)
        // therefore the prior ones will be more specific
        for (int i=0, z=inspectors_.size(); i<z; ++i)
        {
            Alphabet result = inspectors_[i].inspect(bioString);
            if (result != eUnknownAlphabet)
                return result;
        }
    }

    return eUnknownAlphabet;
}

/**
  * First calls AbstractAlphabetDetector::setInspectors() and then calls sortInspectors to update
  * the array of inspectors used to analyze BioStrings.
  *
  * @param inspectors [const QList<AlphabetInspector> &]
  */
void MaxCoverageAlphabetDetector::setInspectors(const QList<AlphabetInspector> &inspectors)
{
    AbstractAlphabetDetector::setInspectors(inspectors);

    qSort(inspectors_.begin(), inspectors_.end(), validatorCoverageLessThan);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private supporting methods specific to this source file
/**
  * @param a [const AlphabetInspector &]
  * @param b [const AlphabetInspector &]
  */
bool validatorCoverageLessThan(const AlphabetInspector &a, const AlphabetInspector &b)
{
    return a.validator().validCharacters().length() < b.validator().validCharacters().length();
}
