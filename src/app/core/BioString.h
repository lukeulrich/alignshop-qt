/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSTRING_H
#define BIOSTRING_H

#include <QtCore/QByteArray>
#include <QtCore/QPair>
#include <QtCore/QVector>
#include <QtCore/QMetaType>

#include "enums.h"
#include "util/ClosedIntRange.h"

/**
  * Generic, normalized character array representing a biological sequence.
  *
  * Biological sequence data originates from many sources and may be represented in diverse forms that make it difficult
  * to readily analyze the raw sequence data. BioString mitigates these issues by producing a normalized version of the
  * actual sequence data during instantiation. Normalization in this sense, simply means the removal of all whitespace
  * characters.
  *
  * BioString may be used to model any biological sequence that may be represented with the basic ASCII indices 32-126.
  * Upon construction, an assertion will be thrown if a character outside this range is encountered. Two variants of
  * static methods provide for converting incompatible strings into an acceptable format, removeUnallowedChars and
  * maskUnallowedChars. The former simply removes these invalid characters, while the latter masks them with a user-
  * defined character.
  *
  * For performance and convenience reasons, BioString is a QByteArray, yet privately inherited so as to better control
  * the interface and prevent corruption by various means (e.g. access hidden methods by casting to QByteArray).
  * BioString contains several utility methods with similar names to those found in the QByteArray class. While these
  * perform largely the same function, the major difference is that BioString is 1-based.
  *
  * Unlike the previous BioStrings design, only absolute positive coordinates may be used to index into BioString.
  * Negative indices are no longer accepted.
  *
  * Operator== performs exact matches and does not equate gap characters.
  */
class BioString : private QByteArray
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    BioString();                                                                //!< Construct an empty BioString
    explicit BioString(Grammar grammar);                                        //!< Construct an empty BioString with grammar
    BioString(const char *str, Grammar grammar = eUnknownGrammar);              //!< Construct a BioString from str and with grammar
    BioString(const QByteArray &byteArray, Grammar grammar = eUnknownGrammar);  //!< Construct a BioString from byteArray and with grammar


    // ------------------------------------------------------------------------------------------------
    // Operators
    BioString &operator+=(const BioString &bioString);                          //!< Append bioString's characters to the end and return a reference to this object; the grammar's must be equivalent
    BioString &operator+=(const QByteArray &byteArray);                         //!< Apend byteArray's characters to the end and return a reference to this object
    BioString &operator+=(const char *str);                                     //!< Append str to the end and return a reference to this object
    BioString &operator+=(char ch);                                             //!< Append ch to the end and return a reference to this object
    bool operator==(const BioString &other) const;                              //!< Returns true if other has an equal grammar and character string; false otherwise
    bool operator==(const QByteArray &byteArray) const;                         //!< Returns true if byteArray is equal to this BioString's characters; false otherwise
    bool operator==(const char *str) const;                                     //!< Returns true if str is equal to this BioString's characters; false otherwise
    bool operator!=(const BioString &other) const;                              //!< Returns true if other has a different grammar or sequence; false otherwise
    bool operator!=(const QByteArray &byteArray) const;                         //!< Returns true if byteArray has a different grammar or sequence; false otherwise
    bool operator!=(const char *str) const;                                     //!< Returns true if str is different than BioString's characters; false otherwise
    BioString &operator=(const QByteArray &byteArray);                          //!< Assign byteArray to this BioString
    BioString &operator=(const char &ch);                                       //!< Assigns ch to this BioString
    BioString &operator=(const char *str);                                      //!< Assigns str to this BioString


    // ------------------------------------------------------------------------------------------------
    // Reimplemented operators
    // The following methods are implemented or strongly based on their base class implementation in QByteArray;
    //   however, they are wrapped using 1-based numbers instead of their zero-based equivalents.
    char operator[](int i) const                                        {   return QByteArray::operator[](i - 1);      }

    // See log.txt, 21 June 2011 for an explanation why this is necessary
    bool operator<(const BioString &other) const                        {   return QByteArray::operator<(other);       }


    // ------------------------------------------------------------------------------------------------
    // Re-exposed QByteArray operators
    // TODO: Fix these! see above
    using QByteArray::operator<;
    using QByteArray::operator<=;
    using QByteArray::operator>;
    using QByteArray::operator>=;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioString &append(const BioString &bioString);                              //!< Append bioString to the end and return a reference to this object; the grammar's must be equivalent
    BioString &append(const char *str);                                         //!< Append str to the end and return a reference to this object
    BioString &append(char ch);                                                 //!< Append ch to the end and return a reference to this object
    QByteArray asByteArray() const;                                             //!< Returns a copy of BioString as a QByteArray
    BioString backTranscribe() const;                                           //!< Returns a DNA grammar copy of BioString with all 'U' and 'u' characters translated to 'T' and 't' characters, respectively
    ClosedIntRange collapseLeft(const ClosedIntRange &range);                   //!< Collapses all characters in range to the left and returns the range of columns changed or empty if none were changed
    ClosedIntRange collapseRight(const ClosedIntRange &range);                  //!< Collapses all characters in range to the right and returns the range of columns changed or empty if none were changed
    BioString complement() const;                                               //!< Returns the DNA complement
    QByteArray digest() const;                                                  //!< Returns the Md5 sequence digest of this BioString
    int gapsBetween(const ClosedIntRange &range) const;                         //!< Returns the number of gaps in range
    int gapsLeftOf(int position) const;                                         //!< Returns the number of contiguous gap characters to the left of the character at position
    int gapsRightOf(int position) const;                                        //!< Returns the number of contiguous gap characters to the right of the character at position
    Grammar grammar() const;                                                    //!< Returns the grammar
    bool hasGapAt(int position) const;                                          //!< Returns true if BioString has a gap at position; false otherwise
    bool hasGaps() const;                                                       //!< Returns true if there is at least one gap present; false otherwise
    bool hasNonGaps() const;                                                    //!< Returns true if there is at least one non-gap character present; false otherwise
    int headGaps() const;                                                       //!< Returns the number of gaps before the first non-gap character
    BioString &insert(int position, const BioString &bioString);                //!< Insert bioString at position (1-based) and return a reference to this object
    BioString &insertGaps(int position, int nGaps, char gapChar);               //!< Insert nGaps gaps at position using gapChar and return a reference to this object
    bool isEquivalentTo(const BioString &other) const;                          //!< Returns true if other has the same grammar, equally positioned gaps characters (but not necessarily identical gap characters), and all non-gap characters are identical in value and position; false otherwise
    bool isExactMatch(int position, const BioString &bioString) const;          //!< Returns true if bioString is an exact substring of this instance beginning at position; false otherwise
    bool isPalindrome() const;                                                  //!< Returns true if bioString has a DnaGrammar, is not empty, consists only of non-gap characters, and is symmetric (i.e. self-complementary); false otherwise
    bool isValidPosition(int position) const;                                   //!< Returns true if position is a valid coordinate contained by BioString; false otherwise
    bool isValidRange(const ClosedIntRange &range) const;                       //!< Returns true if range is not empty and both end points reference valid positions; false otherwise
    int leftSlidablePositions(const ClosedIntRange &range) const;               //!< Returns the number of possible positions that the segment in range may be moved to the left
    BioString mid(const ClosedIntRange &range) const;                           //!< Return the substring referenced by range
    int nonGapsBetween(const ClosedIntRange &range) const;                      //!< Returns the number of non-gap characters in range
    bool onlyContainsACGT() const;                                              //!< Returns true if sequence is not empty and only contains the characters, A, C, G, or T; false otherwise
    bool onlyContainsACGT(const ClosedIntRange &range) const;                   //!< Returns true if the characters in range only contain the characters, A, C, G, or T; false otherwise
    BioString &prepend(const BioString &bioString);                             //!< Prepend bioString to the beginning and return a reference to this object
    BioString &prepend(const char *str);                                        //!< Prepend str to the beginning and return a reference to this object
    BioString &prepend(char ch);                                                //!< Prepend ch to the beginning and return a reference to this object
    BioString &remove(const ClosedIntRange &range);                             //!< Removes the characters in range and returns a reference to this object
    BioString &removeGaps();                                                    //!< Removes all gaps from the sequence and returns a reference to this object
    BioString &removeGaps(int position, int nGaps);                             //!< Remove up to nGaps contiguous gaps beginning with the gap at position, if the character at position is a gap and return a reference to this object
    BioString &replace(int position, int amount, const BioString &bioString);   //!< Replace amount character starting from position (1-based) with bioString and return a reference to this object
    BioString &replace(const ClosedIntRange &range, const BioString &bioString);//!< Replace the characters in range with bioString and return a reference to this object
    BioString &replace(const BioString &before, const BioString &after);        //!< Replace all occurrences of the BioString, before, with the BioString, after, and return a reference to this object
    BioString &reverse();                                                       //!< Reverses the BioString characters
    BioString reverseComplement() const;                                        //!< Returns a reversed, DNA complement ([ATCGatcg] -> [TAGCtagc])
    int rightSlidablePositions(const ClosedIntRange &range) const;              //!< Returns the number of possible positions that the segment in range may be moved to the right
    void setGrammar(Grammar grammar);                                           //!< Sets the grammar to grammar
    int slide(const ClosedIntRange &range, int delta);                          //!< Slide the characters in range up to delta positions and return the number of positions successfully moved
    int tailGaps() const;                                                       //!< Returns the number of gaps occurring after the last non-gap character
    BioString &tr(char before, char after);                                     //!< Replace all occurrences of the character, before, with the character, after, and return a reference to this object
    BioString &tr(const char *query, const char *replacement);                  //!< Character-by-character translation of query characters with replacement characters; returns a reference to this object
    BioString transcribe() const;                                               //!< Returns a RNA grammar copy of BioString with all 'T' and 't' characters translated to 'U' and 'u' characters, respectively
    BioString &translateGaps(char gapChar);                                     //!< Replaces all gap characters with gapChar and return a reference to this object
    BioString ungapped() const;                                                 //!< Returns a copy of BioString with all gap characters removed
    int ungappedLength() const;                                                 //!< Returns the length without counting any gaps


    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    // The following methods are implemented or strongly based on their base class implementation in QByteArray;
    //   however, they are wrapped using 1-based numbers instead of their zero-based equivalents.
    char at(int position) const                                         {   return QByteArray::at(position - 1);                                                    }
    // Counting empty returns 0
    int count(const BioString &bioString) const                         {   return (bioString.isEmpty()) ? 0 : QByteArray::count(bioString.constData());            }
    int indexOf(const BioString &bioString, int from = 1) const         {   int pos = QByteArray::indexOf(bioString.constData(), from - 1);                         return (pos != -1) ? pos + 1 : -1;   }
    int indexOf(const QByteArray &byteArray, int from = 1) const        {   int pos = QByteArray::indexOf(byteArray, from - 1);                                     return (pos != -1) ? pos + 1 : -1;   }
    int indexOf(const QString &string, int from = 1) const              {   int pos = QByteArray::indexOf(string, from - 1);                                        return (pos != -1) ? pos + 1 : -1;   }
    int indexOf(const char *str, int from = 1) const                    {   int pos = QByteArray::indexOf(str, from - 1);                                           return (pos != -1) ? pos + 1 : -1;   }
    int indexOf(char ch, int from = 1) const                            {   int pos = QByteArray::indexOf(ch, from - 1);                                            return (pos != -1) ? pos + 1 : -1;   }
    int lastIndexOf(const BioString &bioString, int from = -1) const    {   int pos = QByteArray::lastIndexOf(bioString.constData(), (from != -1) ? from - 1 : -1); return (pos != -1) ? pos + 1 : -1;   }
    int lastIndexOf(const QByteArray &byteArray, int from = -1) const   {   int pos = QByteArray::lastIndexOf(byteArray, (from != -1) ? from - 1 : -1);             return (pos != -1) ? pos + 1 : -1;   }
    int lastIndexOf(const QString &string, int from = -1) const         {   int pos = QByteArray::lastIndexOf(string, (from != -1) ? from - 1 : -1);                return (pos != -1) ? pos + 1 : -1;   }
    int lastIndexOf(const char *str, int from = -1) const               {   int pos = QByteArray::lastIndexOf(str, (from != -1) ? from - 1 : -1);                   return (pos != -1) ? pos + 1 : -1;   }
    int lastIndexOf(char ch, int from = -1) const                       {   int pos = QByteArray::lastIndexOf(ch, (from != -1) ? from - 1 : -1);                    return (pos != -1) ? pos + 1 : -1;   }
    BioString mid(int position, int len = -1) const                     {   return BioString(QByteArray::mid(position - 1, len), grammar_);                         }
    BioString &remove(int position, int amount)                         {   QByteArray::remove(position - 1, amount); return *this;                                 }
    void truncate(int position)                                         {   QByteArray::truncate(position - 1);                                                     }


    // ------------------------------------------------------------------------------------------------
    // Re-exposed QByteArray public methods
    using QByteArray::capacity;
    using QByteArray::chop;
    using QByteArray::clear;
    using QByteArray::constData;
    using QByteArray::contains;
    using QByteArray::count;
    using QByteArray::endsWith;
    using QByteArray::isEmpty;
    using QByteArray::isNull;
    using QByteArray::left;
    using QByteArray::length;
    using QByteArray::reserve;
    using QByteArray::resize;
    using QByteArray::right;
    using QByteArray::size;
    using QByteArray::split;
    using QByteArray::squeeze;
    using QByteArray::startsWith;
    using QByteArray::toUpper;


    // ------------------------------------------------------------------------------------------------
    // Static methods
    static bool containsUnallowedChars(const QByteArray &byteArray);            //!< Returns true if byteArray contains one or more invalid characters
    //! Returns a QByteArray derived from byteArray with all unallowable (see class description) characters replaced with maskChar
    static QByteArray maskUnallowedChars(const QByteArray &byteArray, char maskChar);
    static QByteArray maskUnallowedChars(const char *str, char maskChar);       //!< Returns a QByteArray of str with all unallowable (see class description) characters replaced with maskChar
    static QByteArray removeUnallowedChars(const QByteArray &byteArray);        //!< Returns a QByteArray derived from byteArray with all unallowable (see class description) characters removed
    static QByteArray removeUnallowedChars(const char *str);                    //!< Returns a QByteArray of str with all unallowable (see class description) characters removed


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void checkString(const char *str, const char *location = "") const;         //!< Scans str invalid characters and throws an assertion with the location string if any are found

    // ------------------------------------------------------------------------------------------------
    // Re-exposed QByteArray methods
    using QByteArray::data;

    // ------------------------------------------------------------------------------------------------
    // Protected members / properties
    Grammar grammar_;

    friend BioString operator+(const BioString &left, const BioString &right);
    friend BioString operator+(const BioString &left, const char *str);
    friend BioString operator+(const char *str, const BioString &right);
    friend BioString operator+(const BioString &left, const QByteArray &byteArray);
    friend BioString operator+(const QByteArray &byteArray, const BioString &right);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    int slideViaSwap(const ClosedIntRange &range, int delta);                   //!< Functionally identical to slide however it utilizes character swapping to achieve the same goal instead of memcpy and memmove
    void safeSlideLeft(const ClosedIntRange &range, int delta);                 //!< Helper method to slideViaSwap
    void safeSlideRight(const ClosedIntRange &range, int delta);                //!< Helper method to slideViaSwap

    static QByteArray swapByteArray_;   // Static storage for the slide method

    friend class TestBioString;
};

Q_DECLARE_METATYPE(BioString)

// ------------------------------------------------------------------------------------------------
// Addition operator function
//! Returns a new BioString whose contents are the combination of left followed by right; both grammars must be equivalent
BioString operator+(const BioString &left, const BioString &right);
//! Returns a new BioString whose contents is the concatenation of left and str with a grammar of left
BioString operator+(const BioString &left, const char *str);
//! Returns a new BioString whose contents is the concatenation of str and right with a grammar of right
BioString operator+(const char *str, const BioString &right);
//! Returns a new BioString whose contents is the concatenation of left and byteArray with a grammar of left
BioString operator+(const BioString &left, const QByteArray &byteArray);
//! Returns a new BioString whose contents is the concatenation of byteArray and right with a grammar of right
BioString operator+(const QByteArray &byteArray, const BioString &right);

#ifndef QT_NO_DEBUG_STREAM
#include <QtDebug>
QDebug operator<<(QDebug debug, const BioString &bioString);
#endif

Q_DECLARE_TYPEINFO(BioString, Q_MOVABLE_TYPE);

typedef QVector<BioString> BioStringVector;
Q_DECLARE_METATYPE(BioStringVector)

#endif // BIOSTRING_H
