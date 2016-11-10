/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef RNASTRING_H
#define RNASTRING_H

#include "BioString.h"

/**
  * Normalized character string of a RNA sequence. (Implicitly shared)
  *
  * This class extends BioString by applying a RNA specific alphabet for both validation and masking of
  * invalid characters. Valid characters include: ABCDGIKMNRSUVWXY*.-
  */
class RnaString : public BioString
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    RnaString(const QString &sequence = QString()); //!< Construct a RnaString from the given sequence (QString)
    RnaString(const char *sequence);                //!< Construct a RnaString from the given sequence (char *)
    RnaString(const RnaString &other);              //!< Construct a copy of other
    virtual RnaString *clone() const;               //!< Virtual copy constructor method
    virtual RnaString *create() const;              //!< Virtual default constructor method


    // ------------------------------------------------------------------------------------------------
    // Destructors
    virtual ~RnaString();                           //!< Virtual destructur (extension friendly :)


    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    // Include all the parent class operator= because there are no additional variables in this class
    using BioString::operator=;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    Alphabet alphabet() const;                      //!< Returns eRnaAlphabet
    using BioString::isValid;
    using BioString::masked;
    virtual QString masked() const;                 //!< Returns the sequence with all invalid characters replaced with RnaString::defaultMaskCharacter_
    using BioString::reduced;
    virtual QString reduced() const;                //!< Returns the sequence with all gaps removed and invalid characters replaced with RnaString::defaultMaskCharacter_

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual bool isValid(const char symbol) const;  //!< Test logic for validating a single symbol

private:
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Construct a RnaString from a QString and normalize
  * @param sequence QString representation of raw sequence data
  * @see normalize()
  */
inline
RnaString::RnaString(const QString &sequence) : BioString(sequence)
{}

/**
  * Construct a RnaString from a character array and normalize
  * @param sequence [const char *] representation of raw sequence data
  * @see normalize()
  */
inline
RnaString::RnaString(const char *sequence) : BioString(sequence)
{}

/**
  * @param other reference to RnaString to be copied
  */
inline
RnaString::RnaString(const RnaString &other) : BioString(other)
{}

/**
  * Artificial virtual copy constructor that allocates an exact copy of the current object.
  *
  * @return RnaString *
  */
inline
RnaString *RnaString::clone() const
{
    return new RnaString(*this);
}

/**
  * Artificial default constructor that creates a new instance of the same type as this.
  *
  * @return RnaString *
  */
inline
RnaString *RnaString::create() const
{
    return new RnaString();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
/**
  * Trivially defined destructor
  */
inline
RnaString::~RnaString()
{}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Return a copy of the sequence with all invalid characters (those that are not ABCDGIKMNRSUVWXY*.-) with the
  * character specified by constants::kRnaMaskCharacter. This differs from BioString::maskInvalidCharacters which
  * directly modifies the internal sequence.
  *
  * @return QString
  * @see BioString::mask()
  */
inline
QString RnaString::masked() const
{
    return BioString::masked(constants::kRnaMaskCharacter);
}

/**
  * Return the sequence with all invalid characters (those that are not ABCDGIKMNRSUVWXY*.-) relplaced with the
  * character specified by constants::kRnaMaskCharacter and all gaps removed. It overrides the virtual function
  * BioString::reduced() to use a different default masking character than that for the default BioString.
  *
  * @return QString
  * @see BioString::reduced()
  */
inline
QString RnaString::reduced() const
{
    return reduced(constants::kRnaMaskCharacter);
}




#endif // RNASTRING_H
