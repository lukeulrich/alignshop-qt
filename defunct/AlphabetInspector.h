/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ALPHABETINSPECTOR_H
#define ALPHABETINSPECTOR_H

#include <QtCore/QString>

#include "global.h"
#include "BioString.h"
#include "BioStringValidator.h"

/**
  * AlphabetInspector inspects a text buffer and returns a known specific Alphabet or Alphabet::UnknownAlphabet
  * if the buffer does not validate with the supplied validator.
  *
  * This class is similar in function to AbstractFormatInspector; however, it differs in that it defines the
  * interface and also provides a concrete implementation.
  *
  * The inspection process is determined by a specific BioStringValidator instance provided during construction.
  * inspect() utilizes the given validator to check whether buffer is valid and if so, returns the Alphabet type
  * supplied during construction. Otherwise, it returns Alphabet::UnknownAlphabet.
  */
class AlphabetInspector
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Constructs an instance of this class with alphabet and validator
    AlphabetInspector(Alphabet alphabet, const BioStringValidator &validator);

    // ------------------------------------------------------------------------------------------------
    // Operators
    //! Returns true if this AlphabetInspector is equivalent to other
    bool operator==(const AlphabetInspector &other);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    Alphabet alphabet() const;                      //!< Returns the associated Alphabet
    //! Returns the constructor-supplied Alphabet type if the bioString is valid according to the constructor-supplied BioStringValidator or Alphabet::UnknownAlphabet otherwise
    Alphabet inspect(const BioString &bioString) const;
    BioStringValidator validator() const;           //!< Returns the associated BioStringValidator instance

private:
    Alphabet alphabet_;                             //!< Alphabet associated with this validator
    BioStringValidator validator_;                  //!< Validator used to check buffer contents for validity
};

#endif // ALPHABETINSPECTOR_H
