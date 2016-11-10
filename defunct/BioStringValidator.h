/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSTRINGVALIDATOR_H
#define BIOSTRINGVALIDATOR_H

#include <QtCore/QString>
#include <QtCore/QScopedPointer>
#include <QtCore/QSharedDataPointer>

#include "BioString.h"


// ------------------------------------------------------------------------------------------------
// Forward declarations
class BioStringValidatorPrivate;       // Defined in the cpp file

/**
  * BioStringValidator provides a flexible mechanism for determining whether a BioString's sequence or arbitrary
  * characters are valid (they are a subset of a user-defined character list).
  *
  * A BioString is considered valid if and only if all of its sequence characters are a subset of validCharacters_.
  * The set of valid characters (passed in as a QString) must be ascii-compatible or else validation may exhibit
  * unexpected behavior.
  *
  * By default, all gap characters (constants::kGapCharacters) are ignored; however, it is possible to change this
  * with the setIgnoreGaps() method.
  *
  * Elected to utilize a char * raw memory location for holding the list of valid characters for both performance
  * and convenience reasons. Specifically, char * is more memory efficient than the QString class. Comparison is
  * also done at the char level, thus, checking for valid characters simply entails iterating over this character
  * array.
  *
  * This class is implicitly shared for convenience and performance reasons.
  */
class BioStringValidator
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct a BioStringValidator and initialize the validator with validCharacters
    explicit BioStringValidator(const QString &validCharacters = QString());
    BioStringValidator(const BioStringValidator &other);                //!< Construct a copy of other

    // ------------------------------------------------------------------------------------------------
    // Destructor
    ~BioStringValidator();                                              //!< Necessary desctructor for implicit sharing

    // ------------------------------------------------------------------------------------------------
    // Operators
    //! Returns true if this BioStringValidator is equivalent to other (same memory location or set of validCharacters)
    bool operator==(const BioStringValidator &other);
    BioStringValidator &operator=(const BioStringValidator &other);     //!< Assigns other to this instance

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QString validCharacters() const;                //!< Returns the string of valid characters used during validation
    bool ignoreGaps() const;                        //!< Returns true if gap characters (constants::kGapCharacters) are ignored during validation; false otherwise
    bool isValid(const BioString &bioString) const; //!< Returns true if all sequence characters in bioString are valid
    bool isValid(const QChar &ch) const;            //!< Returns true if ch is a valid character
    bool isValid(const char &ch) const;             //!< Returns true if ch is a valid character
    void setIgnoreGaps(bool ignoreGaps);            //!< Sets whether to ignore gaps during validation to ignoreGaps
    //! Sets the set of valid characters to validCharacters
    void setValidCharacters(const QString &validCharacters);

private:
    QSharedDataPointer<BioStringValidatorPrivate> d_;
};

#endif // BIOSTRINGVALIDATOR_H


