/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
** Other authors: Paul Ulrich
**
****************************************************************************/

#include "DnaString.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Alphabet
  */
Alphabet DnaString::alphabet() const
{
    return eDnaAlphabet;
}

/**
  * Tests whether the given symbol is a valid DNA character (ABCDGIKMNRSTVWXY*.-).
  * @param symbol [const char] a single character code presumably from a DNA sequence
  * @return bool
  */
bool DnaString::isValid(const char symbol) const
{
    return ((symbol >= 'A' && symbol <= 'D')    // ABCD
            || (symbol >= 'G' && symbol <= 'I') // GHI
            || symbol == 'K'
            || symbol == 'M'
            || symbol == 'N'
            || (symbol >= 'R' && symbol <= 'T') // RST
            || (symbol >= 'V' && symbol <= 'Y') // VWXY
            || symbol == '*'
            || symbol == '.'
            || symbol == '-');
}

/**
  * Coded by Paul Ulrich, 2010-05-11
  *
  * @returns QString
  */
QString DnaString::reverseComplement() const
{
    QString reverseComplement;                  // variable to store new reverse complement sequence
    QString currentSequence = sequence();       // snag the sequence of the object we are reverse complementing!
    int length = currentSequence.length();
    reverseComplement.resize(length);
    const QChar *x = currentSequence.constData();

    for (int i = 0; i< length; ++i)
    {
        if(constants::kDnaBasePair.contains(*x))
            reverseComplement[length - i - 1] = constants::kDnaBasePair[*x];
        else // need to build in error checking here
            reverseComplement[length - i - 1] = *x;
        ++x;
    }

    return reverseComplement;
}

/**
  * Coded by Paul Ulrich, 2010-05-20
  *
  * @returns QString
  */
QString DnaString::reverse() const
{
    QString reversed;                       // variable to store reversed sequence
    QString currentSequence = sequence();   // retrieve the sequence
    int length = currentSequence.length();
    reversed.resize(length);
    const QChar *x = currentSequence.constData();

    for (int i = 0; i< length; ++i)         // iterate through the sequence to reverse it
    {
        reversed[length - i - 1] = *x;
        ++x;
    }

    return reversed;
}
