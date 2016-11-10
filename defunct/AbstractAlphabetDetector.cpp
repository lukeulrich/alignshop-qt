/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AbstractAlphabetDetector.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QList<AlphabetInspector>
  */
QList<AlphabetInspector> AbstractAlphabetDetector::inspectors() const
{
    return inspectors_;
}

/**
  * @param inspectors [const QList<AlphabetInspector> &]
  */
void AbstractAlphabetDetector::setInspectors(const QList<AlphabetInspector> &inspectors)
{
    inspectors_ = inspectors;
}

QList<Alphabet> AbstractAlphabetDetector::detectAlphabets(const QList<BioString> &bioStrings) const
{
    QList<Alphabet> alphabets;

    for (int i=0, z=bioStrings.size(); i<z; ++i)
        alphabets.append(detectAlphabet(bioStrings.at(i)));

    return alphabets;
}
