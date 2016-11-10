/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AminoString.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Alphabet
  */
Alphabet AminoString::alphabet() const
{
    return eAminoAlphabet;
}

/**
  * Return the sequence with all invalid characters replaced with the constants::kAminoMaskCharacter.
  * This method overrides the virtual function BioString::masked() to define the appropriate masking
  * character than that for the default BioString.
  *
  * @return QString
  * @see BioString::masked()
  */
QString AminoString::masked() const
{
    return masked(constants::kAminoMaskCharacter);
}

/**
  * Return the sequence with all invalid characters (those that are not ABCDGIKMNRSTVWXY*.-) relplaced with the
  * character specified by DnaString::defaultMaskCharacter_ and all gaps removed. It overrides the virtual function
  * BioString::reduced() to use a different default masking character than that for the default BioString.
  *
  * @return QString
  * @see BioString::reduced()
  */
QString AminoString::reduced() const
{
    return reduced(constants::kAminoMaskCharacter);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Tests whether the given symbol is a valid amino acid character [A-Z.-]
  * @param symbol [const char] a single character code presumably from an amino acid sequence
  * @return bool
  */
bool AminoString::isValid(const char symbol) const
{
    return ((symbol >= 65 && symbol <= 90) // A-Z
            || symbol == '-'
            || symbol == '.');
}

