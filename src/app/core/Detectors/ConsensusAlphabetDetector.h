/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONSENSUSALPHABETDETECTOR_H
#define CONSENSUSALPHABETDETECTOR_H

#include <QtCore/QHash>

#include "IAlphabetDetector.h"
#include "../Alphabet.h"
#include "../macros.h"

/**
  * Future expansion: add a limit parameter to the detect method that indicates the maximum number of sequences to use
  * when performing the detection process.
  */
template<typename T>
class ConsensusAlphabetDetector
{
public:
    ConsensusAlphabetDetector(IAlphabetDetector *alphabetDetector) : alphabetDetector_(alphabetDetector)
    {
        ASSERT(alphabetDetector_);
    }

    // Note: each element of container must have a sequence() method that returns either a BioString or a type
    //       that can be converted to a BioString
    //
    // If 0, then scan all of entries
    // TODO: Test the limit function
    Alphabet detectConsensusAlphabet(const T &container, int limit = 0) const
    {
        ASSERT(limit >= 0);

        if (container.isEmpty())
            return Alphabet();

        QHash<Alphabet, int> counts;

        typename T::ConstIterator it = container.constBegin();
        for (int n=0; (n == 0 || n < limit) && it != container.constEnd(); ++it, ++n)
            ++counts[alphabetDetector_->detectAlphabet((*it).sequence())];      // <--- Here is the sequence() method!

        // Return the one with the highest value
        QHash<Alphabet, int>::ConstIterator highest = counts.constBegin();
        QHash<Alphabet, int>::ConstIterator hashIterator = counts.constBegin() + 1;
        for (; hashIterator != counts.constEnd(); ++hashIterator)
        {
            if (hashIterator.value() > highest.value())
                // This alphabet occurred more frequently
                highest = hashIterator;
            else if (hashIterator.value() == highest.value() && hashIterator.key() < highest.key())
                // This alphabet occurred just as frequently, but is smaller (and thus more specific)
                highest = hashIterator;
        }

        return highest.key();
    }

private:
    IAlphabetDetector *alphabetDetector_;
};

#endif // CONSENSUSALPHABETDETECTOR_H
