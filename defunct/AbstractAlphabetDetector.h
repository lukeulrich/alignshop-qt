/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTALPHABETDETECTOR_H
#define ABSTRACTALPHABETDETECTOR_H

#include <QtCore/QList>

#include "AlphabetInspector.h"
#include "BioString.h"
#include "global.h"

/**
  * AbstractAlphabetDetector defines the strategy interface for determining the Alphabet of one or more BioStrings.
  *
  * Concrete strategy classes must define the detectAlphabet function which returns a specific and consistent Alphabet
  * that is the most appropriate bioString sequence alphabet. If none can be determined adequately, they should
  * return eUnknownAlphabet.
  *
  * For convenience, detectAlphabets provides a mechanism for determining the alphabet's of several BioStrings.
  */
class AbstractAlphabetDetector
{
public:
    // ------------------------------------------------------------------------------------------------
    // Destructor
    virtual ~AbstractAlphabetDetector();                        //!< Trivial destructor

    // ------------------------------------------------------------------------------------------------
    // Pure virtual methods
    //! Detect and return the alphabet of bioString - interface method to be defined by concrete classes
    virtual Alphabet detectAlphabet(const BioString &bioString) const = 0;

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QList<AlphabetInspector> inspectors() const;                //!< Return the list of alphabet inspectors
    //! Sets the list of alphabet inspectors to inspectors for analyzing BioString(s)
    virtual void setInspectors(const QList<AlphabetInspector> &inspectors);
    //! Convenience method that compiles and returns the results of detectAlphabet for each bioString in bioStrings into a QList<Alphabet>. There is a 1:1 correspondence between QList<BioString> and the returned QList<Alphabet>
    virtual QList<Alphabet> detectAlphabets(const QList<BioString> &bioStrings) const;

protected:
    QList<AlphabetInspector> inspectors_;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructor
/**
  * Trivial inline definition of destructor
  */
inline
AbstractAlphabetDetector::~AbstractAlphabetDetector()
{
}

#endif // ABSTRACTALPHABETDETECTOR_H
