/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "RnaString.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Alphabet
  */
Alphabet RnaString::alphabet() const
{
    return eRnaAlphabet;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Tests whether the given symbol is a valid RNA character (ABCDGIKMNRSUVWXY*.-).
  * @param symbol [const char] a single character code presumably from a RNA sequence
  * @return bool
  */
bool RnaString::isValid(const char symbol) const
{
    return ((symbol >= 'A' && symbol <= 'D')    // ABCD
            || (symbol >= 'G' && symbol <= 'I') // GHI
            || symbol == 'K'
            || symbol == 'M'
            || symbol == 'N'
            || symbol == 'R'
            || symbol == 'S'
            || (symbol >= 'U' && symbol <= 'Y') // UVWXY
            || symbol == '*'
            || symbol == '.'
            || symbol == '-');
}
