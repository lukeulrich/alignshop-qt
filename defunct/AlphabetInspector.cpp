/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AlphabetInspector.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * Assign private members during construction
  */
AlphabetInspector::AlphabetInspector(Alphabet alphabet, const BioStringValidator &validator) :
        alphabet_(alphabet), validator_(validator)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * @param other [const BioStringValidator &]
  * @returns bool
  */
bool AlphabetInspector::operator==(const AlphabetInspector &other)
{
    return this == &other || (alphabet_ == other.alphabet_ &&
                              validator_ == other.validator_);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Alphabet
  */
Alphabet AlphabetInspector::alphabet() const
{
    return alphabet_;
}

/**
  * @param bioString [const BioString &]
  * @returns Alphabet
  */
Alphabet AlphabetInspector::inspect(const BioString &bioString) const
{
    if (validator_.isValid(bioString))
        return alphabet_;

    return eUnknownAlphabet;
}

/**
  * BioStringValidator is implicitly shared, so returning the instance by value is a cheap operation.
  *
  * @returns BioStringValidator
  */
BioStringValidator AlphabetInspector::validator() const
{
    return validator_;
}
