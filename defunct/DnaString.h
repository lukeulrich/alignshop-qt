/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
** Other authors: Paul Ulrich
**
****************************************************************************/

#ifndef DNASTRING_H
#define DNASTRING_H

#include "BioString.h"

#include "global.h"

/**
  * Normalized character string of a DNA sequence. (Implicitly shared)
  *
  * This class extends BioString by applying a DNA specific alphabet for both validation and masking of
  * invalid characters. Valid characters include: ABCDGIKMNRSTVWXY*.-
  */
class DnaString : public BioString
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    DnaString(const QString &sequence = NULL);      //!< Construct a DnaString from the given sequence (QString)
    DnaString(const char *sequence);                //!< Construct a DnaString from the given sequence (char *)
    DnaString(const DnaString &other);              //!< Construct a copy of other
    virtual DnaString *clone() const;               //!< Virtual copy constructor method
    virtual DnaString *create() const;              //!< Virtual default constructor method

    // ------------------------------------------------------------------------------------------------
    // Destructors
    virtual ~DnaString();                           //!< Virtual destructur (extension friendly :)

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    // Include all the parent class operator= because there are no additional variables in this class
    using BioString::operator=;

    // ------------------------------------------------------------------------------------------------
    // Public methods
    Alphabet alphabet() const;                      //!< Returns eDnaAlphabet
    using BioString::isValid;
    using BioString::masked;
    virtual QString masked() const;                 //!< Returns the sequence with all invalid characters replaced with DnaString::defaultMaskCharacter_
    using BioString::reduced;
    virtual QString reduced() const;                //!< Returns the sequence with all gaps removed and invalid characters replaced with DnaString::defaultMaskCharacter_

    QString reverseComplement() const;              //!< Returns the reverse complement of sequence
    QString reverse() const;                        //!< Returns simply the reversed sequence

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual bool isValid(const char symbol) const;  //!< Test logic for validating a single symbol
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Construct a DnaString from a QString and normalize
  *
  * @param sequence QString representation of raw sequence data
  * @see normalize()
  */
inline
DnaString::DnaString(const QString &sequence) : BioString(sequence)
{}

/**
  * Construct a DnaString from a character array and normalize
  *
  * @param sequence [const char *] representation of raw sequence data
  * @see normalize()
  */
inline
DnaString::DnaString(const char *sequence) : BioString(sequence)
{}

/**
  * @param other reference to DnaString to be copied
  */
inline
DnaString::DnaString(const DnaString &other) : BioString(other)
{}

/**
  * Artificial virtual copy constructor that allocates an exact copy of the current object.
  *
  * @returns new DnaString object
  */
inline
DnaString *DnaString::clone() const
{
    return new DnaString(*this);
}

/**
  * Artificial default constructor that allocates a new default instance of the same type as this.
  *
  * @returns DnaString *
  */
inline
DnaString *DnaString::create() const
{
    return new DnaString();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
/**
  * Trivially defined destructor
  */
inline
DnaString::~DnaString()
{}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Return the sequence with all invalid characters (those that are not ABCDGIKMNRSTVWXY*.-) replaced with the
  * character specified by DnaString::defaultMaskCharacter_. It overrides the virtual function BioString::masked()
  * to use a different default masking character than that for the default BioString.
  *
  * @return QString
  * @see BioString::masked()
  */
inline
QString DnaString::masked() const
{
    return masked(constants::kDnaMaskCharacter);
}

/**
  * Return the sequence with all invalid characters (those that are not ABCDGIKMNRSTVWXY*.-) relplaced with the
  * character specified by DnaString::defaultMaskCharacter_ and all gaps removed. It overrides the virtual function
  * BioString::reduced() to use a different default masking character than that for the default BioString.
  *
  * @return QString
  * @see BioString::reduced()
  */
inline
QString DnaString::reduced() const
{
    return reduced(constants::kDnaMaskCharacter);
}

#endif // DNASTRING_H
