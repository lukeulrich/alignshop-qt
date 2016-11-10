/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "BioStringValidator.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param validCharacters [const BioString &]
  */
BioStringValidator::BioStringValidator(const BioString &validCharacters)
{
    setValidCharacters(validCharacters);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operators
/**
  * @param other [const BioStringValidator &]
  * @returns bool
  */
bool BioStringValidator::operator==(const BioStringValidator &other) const
{
    return validCharSet_ == other.validCharSet_;
}

/**
  * @param other [const BioStringValidator &]
  * @returns bool
  */
bool BioStringValidator::operator!=(const BioStringValidator &other) const
{
    return !operator==(other);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param bioString [const BioString &]
  * @returns bool
  */
bool BioStringValidator::isValid(const BioString &bioString) const
{
    const char *x = bioString.constData();

    for (; *x; ++x)
        if (!validCharSet_.contains(*x))
            return false;

    return true;
}

/**
  * @param bioString [const BioString &]
  * @returns bool
  */
bool BioStringValidator::isValid(const QByteArray &byteArray) const
{
    const char *x = byteArray.constData();

    for (; *x; ++x)
        if (!validCharSet_.contains(*x))
            return false;

    return true;
}

/**
  * @param ch [const QChar &]
  * @returns bool
  */
bool BioStringValidator::isValid(const QChar &ch) const
{
    return validCharSet_.contains(ch.toAscii());
}

/**
  * @param ch [const char &]
  * @returns bool
  */
bool BioStringValidator::isValid(const char &ch) const
{
    return validCharSet_.contains(ch);
}

/**
  * @returns BioString
  */
BioString BioStringValidator::validCharacters() const
{
    BioString bioString;
    bioString.reserve(validCharSet_.size());

    QSet<char>::ConstIterator it = validCharSet_.constBegin();
    for (; it != validCharSet_.constEnd(); ++it)
        bioString.append(*it);

    return bioString;
}

/**
  * @param validCharacters [const BioString &]
  */
void BioStringValidator::setValidCharacters(const BioString &validCharacters)
{
    const char *x = validCharacters.constData();
    while (*x)
    {
        validCharSet_ << *x;
        ++x;
    }
}
