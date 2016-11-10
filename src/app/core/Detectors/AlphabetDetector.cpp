/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AlphabetDetector.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
AlphabetDetector::AlphabetDetector(QObject *parent)
    : IAlphabetDetector(parent), acceptAnyCharacter_(true)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool AlphabetDetector::acceptAnyCharacter() const
{
    return acceptAnyCharacter_;
}

/**
  * At least one non-gap character must be present for a non-default alphabet to be returned.
  *
  * @param bioString [const BioString &]
  * @returns Alphabet
  */
Alphabet AlphabetDetector::detectAlphabet(const BioString &bioString) const
{
    // Since the alphabet are sorted in ascending order, the most specific alphabet is the first one that passes
    // the validation.
    QVector<BioStringValidator>::ConstIterator it = validators_.constBegin();
    for (; it != validators_.constEnd(); ++it)
        if (bioString.length() > 0 && (*it).isValid(bioString) && bioString.hasNonGaps())
            return alphabets_.at(it - validators_.constBegin());

    // None of the validators passed
    return Alphabet();
}

/**
  * @param acceptAnyCharacter [bool]
  */
void AlphabetDetector::setAcceptAnyCharacter(bool acceptAnyCharacter)
{
    if (acceptAnyCharacter_ == acceptAnyCharacter)
        return;

    acceptAnyCharacter_ = acceptAnyCharacter;

    int i = 0;
    QVector<BioStringValidator>::Iterator it = validators_.begin();
    for (; it != validators_.end(); ++it, ++i)
        (*it).setValidCharacters((acceptAnyCharacter_) ? alphabets_.at(i).allCharacters() : alphabets_.at(i).characters());
}

/**
  * @param alphabets [const QVector<Alphabet] &]
  */
void AlphabetDetector::setAlphabets(const QVector<Alphabet> &alphabets)
{
    alphabets_ = alphabets;
    qStableSort(alphabets_);
    validators_.resize(alphabets_.size());

    int i = 0;
    foreach (const Alphabet &alphabet, alphabets_)
    {
        QByteArray validCharacters = (acceptAnyCharacter_) ? alphabet.allCharacters() : alphabet.characters();
        validCharacters += constants::kGapCharacters;
        validators_[i].setValidCharacters(validCharacters);
        ++i;
    }
}

/**
  * @param alphabet [const Alphabet &]
  * @returns BioStringValidator
  */
BioStringValidator AlphabetDetector::validatorForAlphabet(const Alphabet &alphabet) const
{
    int index = alphabets_.indexOf(alphabet);
    if (index != -1)
        return validators_.at(index);

    return BioStringValidator();
}
