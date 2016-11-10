/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSTRING_H
#define AMINOSTRING_H

#include "BioString.h"

/**
  * Normalized character string of an amino acid sequence. (Implicitly shared)
  *
  * This class extends BioString by applying an amino acid specific alphabet for both validation and masking of
  * invalid characters. Because the default mask character for BioString (i.e. X) is the same as that used for
  * AminoStrings, it is not necessary to override the sequenceMasked(...) and maskInvalidCharacters(...) methods.
  */
class AminoString : public BioString
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    AminoString(const QString &sequence = QString()); //!< Construct a AminoString from the given sequence (QString)
    AminoString(const char *sequence);                //!< Construct a AminoString from the given sequence (char *)
    AminoString(const AminoString &other);            //!< Construct a copy of other
    virtual AminoString *clone() const;               //!< Virtual, copy constructor method
    virtual AminoString *create() const;              //!< Virtual, default constructor method


    // ------------------------------------------------------------------------------------------------
    // Destructors
    virtual ~AminoString();                           //!< Virtual destructor (extension friendly :)


    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    // Include all the parent class operator= because there are no additional variables in this class
    using BioString::operator=;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    Alphabet alphabet() const;                        //!< Returns eAminoAlphabet

    // We want to have the isValid function from the BioString class, but the single isValid(const char)
    // function defined in the protected section effectively hides all other isValid(...) calls that may
    // be present in the base class. To use both those defined in the base class and this one, we have to
    // tell the compiler to also look in the base class
    using BioString::isValid;
    using BioString::masked;
    virtual QString masked() const;                   //!< Returns the sequence with all invalid characters replaced with constants::kAminoMaskCharacter
    using BioString::reduced;
    virtual QString reduced() const;                  //!< Returns the sequence with all gaps removed and invalid characters replaced with constants::kAminoMaskCharacter

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual bool isValid(const char symbol) const;    //!< Test logic for validating a single symbol

private:
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Construct a AminoString from a QString and normalize
  * @param sequence QString representation of raw sequence data
  * @see normalize()
  */
inline
AminoString::AminoString(const QString &sequence) : BioString(sequence)
{}

/**
  * Construct a AminoString from a character array and normalize
  * @param sequence [const char *] representation of raw sequence data
  * @see normalize()
  */
inline
AminoString::AminoString(const char *sequence) : BioString(sequence)
{}

/**
  * @param other reference to AminoString to be copied
  */
inline
AminoString::AminoString(const AminoString &other) : BioString(other)
{}

/**
  * Artificial virtual copy constructor that allocates an exact copy of the current object.
  *
  * @returns AminoString *
  */
inline
AminoString *AminoString::clone() const
{
    return new AminoString(*this);
}

/**
  * Artificial default constructor that allocates a new default instance of the same type as this.
  *
  * @returns AminoString *
  */
inline
AminoString *AminoString::create() const
{
    return new AminoString();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
/**
  * Trivially defined destructor
  */
inline
AminoString::~AminoString()
{}


#endif // AMINOSTRING_H
