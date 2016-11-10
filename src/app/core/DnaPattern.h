/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNAPATTERN_H
#define DNAPATTERN_H

#include <QtCore/QByteArray>

class BioString;

/**
  * DnaPattern encapsulates basic DNA pattern matching using a simple mapping of various combinations of the DNA
  * alphabet to single characters.
  *
  * The mapping of characters used in this class is taken from the IUPAC standards:
  * http://www.chem.qmul.ac.uk/iubmb/misc/naseq.html
  * DNA
  * Table 1. Summary of single-letter code recommendations
  * Symbol	Meaning             Origin of designation
  * G        G                   Guanine
  * A        A                   Adenine
  * T        T                   Thymine
  * C        C                   Cytosine
  * R        G or A              puRine
  * Y        T or C              pYrimidine
  * M        A or C              aMino
  * K        G or T              Keto
  * S        G or C              Strong interaction (3 H bonds)
  * W        A or T              Weak interaction (2 H bonds)
  * H        A or C or T         not-G, H follows G in the alphabet
  * B        G or T or C         not-A, B follows A
  * V        G or C or A         not-T (not-U), V follows U
  * D        G or A or T         not-C, D follows C
  * N        G or A or T or C	 aNy of the above (does not match just any character)
  *
  * Additionally, the following rules are included:
  * -        . or -              gap character
  * [space]                      Any character whatsoever
  *
  * All matching is done in a case-insensitive manner, but the pattern input is case-sensitive.
  *
  * Because the user may set the pattern to any arbitrary string, it is quite possible to set an invalid pattern that
  * contains invalid symbols. The validity of a pattern may be checked with the isValid() and isValidPattern() methods.
  * Invalid and empty patterns never return a positive match. Note, input BioStrings do not need to have a DnaGrammar.
  */
class DnaPattern
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    DnaPattern(const QByteArray &pattern = QByteArray());   //!< Constructs an instance with pattern


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void clear();                                           //!< Clears the pattern
    QString displayText() const;                            //!< Returns a more human friendly, readable representation of the underlying DNA pattern
    //! Attempts to find a match in bioString starting at offset (1 by default); returns the position of the first match (1-based coordinates) or -1 if none is found.
    int indexIn(const BioString &bioString, const int offset = 1) const;
    bool isEmpty() const;                                   //!< Returns true if the pattern is empty; false otherwise
    bool isValid() const;                                   //!< Returns true if this pattern is valid; false otherwise
    bool isValidPattern(const QByteArray &pattern) const;   //!< Returns true if pattern is valid; false otherwise
    int length() const;                                     //!< Returns the length of this pattern
    //! Returns true if pattern is found in bioString at offset (1-based); false otherwise
    bool matchesAt(const BioString &bioString, const int offset) const;
    //! Returns ture if pattner is found at the beginning of bioString; false otherwise
    bool matchesAtBeginning(const BioString &bioString) const;
    bool matchesAtEnd(const BioString &bioString) const;    //!< Returns true if pattern is found at the end of bioString; false otherwise
    QByteArray pattern() const;                             //!< Returns the pattern
    void setPattern(const QByteArray &newPattern);          //!< Sets the pattern to newPattern; if newPattern consists of invalid symbols isValid() will return false


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! Returns true if queryChar matches patternChar; false otherwise
    bool matches(const char queryChar, const char patternChar) const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    QByteArray pattern_;        //!< The underlying pattern
    bool valid_;                //!< Flag if this pattern is valid
};

Q_DECLARE_TYPEINFO(DnaPattern, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(DnaPattern)

#endif // DNAPATTERN_H
