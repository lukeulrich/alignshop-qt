/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "BioStringValidator.h"

#include <QDebug>

#include "global.h"

/**
  * Private class used to make BioStringValidator implicitly shared.
  *
  * Implicitly shared class requires three functions to be defined:
  * 1. Default constructor
  * 2. Copy constructor
  * 3. Destructor
  *    All these must be defined even if trivial placeholders
  */
class BioStringValidatorPrivate : public QSharedData
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    BioStringValidatorPrivate();
    BioStringValidatorPrivate(const BioStringValidatorPrivate &other);

    // ------------------------------------------------------------------------------------------------
    // Destructor
    ~BioStringValidatorPrivate();

    // ------------------------------------------------------------------------------------------------
    // Convenience functions
    void copyValidCharacters(const QScopedPointer<char> &validCharacters);

    // ------------------------------------------------------------------------------------------------
    // Since this is a private class, publicly expose the member variables
    bool ignoreGaps_;
    QScopedPointer<char> validCharacters_;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Trivial default constructor
  */
BioStringValidatorPrivate::BioStringValidatorPrivate()
{}

/**
  * Perform deep copy of alphabet_ if it is not invalid
  *
  * @param other [const BioStringValidatorPrivate &]
  */
BioStringValidatorPrivate::BioStringValidatorPrivate(const BioStringValidatorPrivate &other) : QSharedData(other), ignoreGaps_(other.ignoreGaps_)
{
    copyValidCharacters(other.validCharacters_);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructor
/**
  * Trivial destructor
  */
BioStringValidatorPrivate::~BioStringValidatorPrivate()
{}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Convenience functions
void BioStringValidatorPrivate::copyValidCharacters(const QScopedPointer<char> &validCharacters)
{
    if (!validCharacters.isNull())
    {
        // Deep-copy of the alphabet_
        validCharacters_.reset(new char[strlen(validCharacters.data()) + 1]);
        qstrcpy(validCharacters_.data(), validCharacters.data());
    }
}


// ************************************************************************************************
// ------------------------------------------------------------------------------------------------
// END PRIVATE CLASS
// ------------------------------------------------------------------------------------------------
// ************************************************************************************************


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param alphabet [const QString &]
  */
BioStringValidator::BioStringValidator(const QString &validCharacters) : d_(new BioStringValidatorPrivate())
{
    // By default we ignore gap characters during validation
    d_->ignoreGaps_ = true;
    setValidCharacters(validCharacters);
}

/**
  * @param other [const BioStringValidator &]
  */
BioStringValidator::BioStringValidator(const BioStringValidator &other) : d_(other.d_)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructor
/**
  * Trivial destructor
  */
BioStringValidator::~BioStringValidator()
{}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * @param other [const BioStringValidator &]
  * @returns bool
  */
bool BioStringValidator::operator==(const BioStringValidator &other)
{
    return d_ == other.d_ || strcmp(d_->validCharacters_.data(), other.d_->validCharacters_.data()) == 0;
}

/**
  * @param other [const BioStringValidator &]
  * @returns BioStringValidator &
  */
BioStringValidator &BioStringValidator::operator=(const BioStringValidator &other)
{
    // Gracefully handle self-assignment
    if (this == &other)
        return *this;

    d_->copyValidCharacters(other.d_->validCharacters_);
    d_->ignoreGaps_ = other.d_->ignoreGaps_;

    return *this;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Transforms the alphabet_ character array into a QString object.
  *
  * @returns QString
  */
QString BioStringValidator::validCharacters() const
{
    return QString(d_->validCharacters_.data());
}

/**
  * @returns bool
  */
bool BioStringValidator::ignoreGaps() const
{
    return d_->ignoreGaps_;
}

/**
  * @param bioString [const BioString &]
  * @returns bool
  */
bool BioStringValidator::isValid(const BioString &bioString) const
{
    const QChar *x = bioString.sequence().constData() - 1;

    while (*(++x) != '\0')
        if (!isValid((*x).toAscii()))        // Shortcut to avoid having an extra function call
            return false;

    return true;
}

/**
  * @param ch [const QChar &]
  * @returns bool
  */
bool BioStringValidator::isValid(const QChar &ch) const
{
    return isValid(ch.toAscii());
}

/**
  * @param ch [const char &]
  * @returns bool
  */
bool BioStringValidator::isValid(const char &ch) const
{
    // Loop through every character in validCharacters_ and return true if a match is found
    const char *x = d_->validCharacters_.data() - 1;
    while (*(++x))
        if (*x == ch)
            return true;

    if (d_->ignoreGaps_)
    {
        x = constants::kGapCharacters - 1;
        while (*(++x) != '\0')
        {
            if (*x == ch)
                return true;
        }
    }

    // ch did not match any character in the provided character list
    return false;
}

/**
  * @param ignoreGaps [bool]
  */
void BioStringValidator::setIgnoreGaps(bool ignoreGaps)
{
    d_->ignoreGaps_ = ignoreGaps;
}

/**
  * Copies the characters in alphabet to a heap-allocated character array for faster comparison during validation.
  *
  * @param alphabet [const QString &]
  */
void BioStringValidator::setValidCharacters(const QString &validCharacters)
{
    d_->validCharacters_.reset(new char[validCharacters.length() + 1]);
    qstrcpy(d_->validCharacters_.data(), validCharacters.toAscii().constData());
}
