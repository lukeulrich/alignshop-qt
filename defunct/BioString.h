/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSTRING_H
#define BIOSTRING_H

#include <QtCore/QString>

#include "global.h"

/**
  * Generic, normalized (see detailed description) character string of a biological sequence. (Implicitly shared)
  *
  * Provides both an abstract and concrete interface for tidying up biological sequences (here represented as
  * QStrings) for further analysis. Biological sequence data originates from many sources and may be represented in
  * diverse forms that make it difficult to readily analyze the raw sequence data. The BioString class mitigates these
  * issues by merely storing a normalized version of the actual sequence data, which occurs during instantiation.
  * Normalization in this sense, simply means the removal of all whitespace and making all character upper-case.
  *
  * BioString may be used to model any biological sequence; however, it will only be considered valid if all
  * character data is one of the following: A-Z, *, -, or . Specific BioString derivates (e.g. AminoStrings, etc.)
  * may implement additional validation logic.
  *
  * Several utility functions provide a single-point for common sequence manipulations. There are two versions
  * for each utility function. One operates on the object itself and the other provides a static method for performing
  * this action on any external QString. Because the normalize method will be called automatically upon construction
  * and whenever the sequence changes or is updated, it is not necessary to expose this as a public method. Any
  * calls to normalize are possible via the static normalize(QString) method.
  *
  * BioString contains several utility methods with similar names to those found in the QString class. While these
  * perform largely the same function, the major difference is that BioString is 1-based and QString is 0-based. This
  * can lead to other minor differences such as the indexOf function returns 0 (instead of -1) if a match is not found.
  */
class BioString
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    BioString(const QString &sequence = QString());         //!< Construct a BioString from the given sequence (QString)
    BioString(const char *sequence);                        //!< Construct a BioString from the given sequence (char *)
    BioString(const BioString &other);                      //!< Construct a copy of other
    virtual BioString* clone() const;                       //!< Virtual copy constructor
    virtual BioString* create() const;                      //!< Virtual default constructor


    // ------------------------------------------------------------------------------------------------
    // Destructors
    virtual ~BioString();                                   //!< Virtual because BioString is base class interface


    // ------------------------------------------------------------------------------------------------
    // Operators
    virtual BioString &operator=(const QString &sequence);  //!< Assigns sequence to this BioString
    virtual BioString &operator=(const char *sequence);     //!< Assigns sequence to this BioString
    virtual BioString &operator=(const char &symbol);       //!< Assigns symbol to this BioString
    virtual BioString &operator=(const QChar &symbol);      //!< Assigns symbol to this BioString
    virtual BioString &operator=(const BioString &other);   //!< Assign other to this BioString

    bool operator!=(const BioString &other) const;          //!< Returns true if other does not have an equal sequence and equally placed gaps; otherwise returns false
    BioString &operator+=(const BioString &bioString);      //!< Append bioString to the end of this BioString and return a reference to this object
    bool operator==(const BioString &other) const;          //!< Return true if both BioStrings have equal sequences and equally placed gaps; otherwise returns false
    const QChar operator[](int i) const;                    //!< Return the const character at the specified position, i (1-based)

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual Alphabet alphabet() const;                              //!< Returns the type of alphabet of this sequence
    BioString &append(const BioString &bioString);                  //!< Append bioString to the end of this BioString and return a reference to this object
    bool hasGaps() const;                                           //!< Test for gap characters [.-] in sequence
    bool hasCharacters() const;                                     //!< Test for non-gap characters in sequence
    int gapsLeftOf(int position) const;                             //!< Returns the number of contiguous gap characters to the left of the character referenced at position
    int gapsRightOf(int position) const;                            //!< Returns the number of contiguous gap characters to the right of the character referenced at position
    int indexOf(const BioString &bioString, int from = 1) const;    //!< Returns the index position (1-based) of the first occurrence of bioString in this BioString searching forward from from. Returns 0 if bioString is not found
    BioString &insert(int position, const BioString &bioString);    //!< Insert bioString at the given index position (1-based) and return a reference to this object

    //! Insert n gaps at RelAbs position using gapCharacter and return a reference to this object
    BioString &insertGaps(int position, int n, char gapCharacter = constants::kDefaultGapCharacter);
    virtual bool isValid() const;                                   //!< Returns true if instance only contains valid characters
    int leftSlidablePositions(int start, int stop) const;           //!< Returns the number of possible positions that the segment from start to stop may be moved to the left
    int length() const;                                             //!< Return the full length of the sequence

    // Note: Unable to simply define a single masked function with a default value because the masked function is virtual. Thus, we emulate this effect by
    // also defining a masked function that takes no arguments
    QString masked(const char &maskCharacter) const;                //!< Returns the sequence with all invalid characters replaced with maskCharacter
    virtual QString masked() const;                                 //!< Returns the sequence with all invalid characters replaced with BioString::defaultMaskCharacter_
    QString mid(int start, int n = 1) const;                        //!< Return a substring that contains n characters beginning at the specified start index (1-based, inclusive)
    int negativeIndex(const int &index) const;                      //!< Return the negative index (1-based) that corresponds to index with respect to the sequence_ length
    int nonGapCharsBetween(int start, int stop) const;              //!< Returns the number of non-gap characters between start and stop (1-based, inclusive)
    int positiveIndex(const int &index) const;                      //!< Return the positive index (1-based) that corresponds to index with respect to the sequence_ length
    BioString &prepend(const BioString &bioString);                 //!< Prepend bioString to the beginning of this BioString and return a reference to this object

    // Note: Unable to simply define a single reduced function with a default value because the reduced function is virtual. Thus, we emulate this effect by
    // also defining a reduced function that takes no arguments
    QString reduced(const char &maskCharacter) const;               //!< Returns the sequence without gaps and invalid characters masked with maskCharacter
    virtual QString reduced() const;                                //!< Returns the sequence without gaps and invlaid characters masked with BioString::defaultMaskCharacter_
    BioString &remove(int position, int n);                         //!< Remove n characters starting from the given index position (1-based) and return a reference to this object
    BioString &removeGaps(int position, int n);                     //!< Remove up to n contiguous gaps if the character at position is a gap
    //! Replace n symbols starting from position (1-based) with replacement
    BioString &replace(int position, int n, const BioString &replacement);
    //! Replace the segment between start and stop (1-based) with replacement
    BioString &replaceSegment(int start, int stop, const BioString &replacement);
    int rightSlidablePositions(int start, int stop) const;          //!< Returns the number of possible positions that the segment from start to stop may be moved to the right
    QString segment(int start = 0, int stop = 0) const;             //!< Return a substring between start and end (1-based, inclusive), which both may be expressed in RelAbs coordinates
    QString sequence() const;                                       //!< Returns the sequence
    int slideSegment(int start_pos, int stop_pos, int delta);       //!< Slide the characters between start_pos and stop_pos up to delta positions and return the number of positions successfully moved
    QString substituteGapsWith(char ch) const;                      //!< Returns a copy of the sequence with all gap characters replaced by ch
    virtual QString ungapped() const;                               //!< Returns a copy of the sequence without gaps


    // ------------------------------------------------------------------------------------------------
    // Static methods
    static bool isGap(const QChar &symbol);                         //!< Return true if char is a gap
    static QString normalize(QString sequence);                     //!< Remove all whitespace and uppercase characters
    static QString normalize(const QChar &symbol);                  //!< Remove any whitespace and uppercase character

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual bool isValid(const char symbol) const;                  //!< Tests whether symbol is a valid sequence character
    virtual bool isValid(const QChar &symbol) const;                //!< Identical to isValid(const char) except applies to QChar

private:
    QString sequence_;                                              //!< Raw sequence underlying this BioString
};


// ------------------------------------------------------------------------------------------------
// Addition operator function
//! Returns a new QString whose contents are the combination of left followed by right
QString operator+(const BioString &left, const BioString &right);



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Construct a BioString from a QString and normalize
  * @param sequence QString representation of raw sequence data
  * @see normalize()
  */
inline
BioString::BioString(const QString &sequence)
{
    sequence_ = normalize(sequence);
}

/**
  * Construct a BioString from a character array and normalize
  * @param sequence [const char *] representation of raw sequence data
  * @see normalize()
  */
inline
BioString::BioString(const char *sequence)
{
    sequence_ = normalize(sequence);
}

/**
  * @param other reference to BioString to be copied
  */
inline
BioString::BioString(const BioString &other) : sequence_(other.sequence_)
{}

/**
  * Virtual copy constructor that allocates an exact copy of the current object.
  * Derived classes should override this function to instantiate an instance of the proper
  * class when client classes call this function via a base class (BioString) pointer.
  *
  * Owernship is passed to the client.
  *
  * @returns BioString *
  */
inline
BioString *BioString::clone() const
{
    return new BioString(*this);
}

/**
  * Virtual default constructor: create a new covariant BioString instance from this instance.
  * Derived classes should override this function to instantiate an instance of the proper
  * class when client classes call this function via a base class pointer (BioString).
  *
  * Owernship is passed to the client.
  *
  * @returns BioString *
  */
inline
BioString *BioString::create() const
{
    return new BioString();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
/**
  * Trivially defined destructor
  */
inline
BioString::~BioString()
{}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * Assigns the normalized sequence to the current BioString
  * @param sequence [QString] new BioString sequence
  */
inline
BioString &BioString::operator=(const QString &sequence)
{
    sequence_ = normalize(sequence);

    return *this;
}

/**
  * Assigns the character array as a normalized sequence to the current BioString
  * @param sequence [const char *] new BioString sequence
  */
inline
BioString &BioString::operator=(const char *sequence)
{
    sequence_ = normalize(sequence);

    return *this;
}

/**
  * Assigns the character symbol as a normalized sequence to the current BioString
  * @param sequence [const char] new BioString sequence
  */
inline
BioString &BioString::operator=(const char &symbol)
{
    sequence_ = normalize(symbol);

    return *this;
}

/**
  * Assigns the character array as a normalized sequence to the current BioString
  * @param sequence [const char *] new BioString sequence
  */
inline
BioString &BioString::operator=(const QChar &symbol)
{
    sequence_ = normalize(symbol);

    return *this;
}

/**
  * Assigns other to the current object
  * @param other reference to BioString to be assigned
  */
inline
BioString &BioString::operator=(const BioString &other)
{
    // Gracefully handle self-assignment
    if (this == &other)
        return *this;

    sequence_ = other.sequence_;

    return *this;
}

/**
  * Two BioStrings are unequal if they have differing sequence data and/or gap placement
  * @param other [const BioString &]
  * @return bool
  * @see operator==()
  */
inline
bool BioString::operator!=(const BioString &other) const
{
    return !operator==(other);
}

/**
  *
 Example:
  * BioString x = "ABC";
  * BioString y = "DEF";
  * x += y;
  * // x == "ABCDEF"
  * x = "ABC";
  * x += "DE";
  * x += "F";
  * // x == "ABCDEF"
  * @param bioString [const BioString &]
  * @return BioString &
  */
inline
BioString &BioString::operator+=(const BioString &bioString)
{
    return append(bioString);
}

/**
  * Two BioStrings are equivalent if they are identical with respect to non-gap characters and gap placement.
  * Example:
  * BioString x = "ABC...DEF";
  * x == "ABC---DEF";   // true
  * x == "-ABC...DEF";  // false
  * x == "AB----DEF";   // false
  * @param other [const BioString &]
  * @return bool
  */
inline
bool BioString::operator==(const BioString &other) const
{
    if (sequence_.length() != other.sequence_.length())
        return false;

    // To make comparison independent of the gap character, simply make a copy with all gaps represented as dashes
    // and compare the QStrings.
    QString subject = sequence_;
    QString query = other.sequence_;

    subject.replace(".", "-");
    query.replace(".", "-");

    return subject == query;
}

/**
  * Position i must refer to a valid index (1-based)
  *
  * @param i [int]
  * @return const QChar
  */
inline
const QChar BioString::operator[](int i) const
{
    Q_ASSERT_X(i != 0, "BioString::operator[]", "Index may not equal 0");

    i = positiveIndex(i);

    Q_ASSERT_X(i > 0 && i <= sequence_.length(), "BioString::operator[]", "index out of range");

    return sequence_.at(i-1);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public inline methods
/**
  * @returns Alphabet
  */
inline
Alphabet BioString::alphabet() const
{
    return eUnknownAlphabet;
}

/**
  * Example:
  * BioString x = "ABC";
  * BioString y = "DEF";
  * x.append(y);
  * // x == "ABCDEF"
  * x = "ABC";
  * x.append("DE").append("F");
  * // x == "ABCDEF"
  * @param bioString [const BioString &]
  * @return BioString &
  */
inline
BioString &BioString::append(const BioString &bioString)
{
    sequence_.append(bioString.sequence_);

    return *this;
}

/**
  * Checks for the presence of non-gap characters
  * @return bool
  */
inline
bool BioString::hasCharacters() const
{
    const QChar *x = sequence_.constData();
    while (*x != '\0')
    {
        if (!BioString::isGap(*x))
            return true;
        ++x;
    }

    return false;
}

/**
  * Checks for the presence of gaps (- and/or .)
  * @return bool
  */
inline
bool BioString::hasGaps() const
{
    return sequence_.contains('-') || sequence_.contains('.');

}

/**
  * @return int length of sequence
  */
inline
int BioString::length() const
{
    return sequence_.length();
}

/**
  * Return a copy of the sequence with all invalid characters replaced with BioString::defaultMaskCharacter_.
  * This function is also declared so that derived classes may perform masking using the appropriate defaultMaskCharacter
  * @return QString the masked sequence
  */
inline
QString BioString::masked() const
{
    return masked(constants::kGenericBioStringMaskCharacter);
}

/**
  * Utility function strictly used for translating indices to their negative position (relative to the right or C-terminal
  * sequence boundary). This is frequently used in the segment(...), mid(...), and other functions when working with
  * either positive or negative coordinates. If for some reason, zero is passed to this function, zero is returned.
  * @param index [const int &]
  * @return int
  * @see positiveIndex(), segment(), mid()
  */
inline
int BioString::negativeIndex(const int &index) const
{
    if (index != 0)
      return (index > 0) ? index - sequence_.length() - 1 : index;

    return 0;
}

/**
  * Utility function strictly used for translating indices to their positive position (relative to the left or N-terminal
  * sequence boundary). This is frequently used in the segment(...), mid(...), and other functions when working with
  * either positive or negative coordinates. If for some reason, zero is passed to this function, zero is returned.
  * @param index [const int &]
  * @return int
  * @see negativeIndex(), segment(), mid()
  */
inline
int BioString::positiveIndex(const int &index) const
{
    if (index != 0)
        return (index < 0) ? index + sequence_.length() + 1 : index;

    return 0;
}

/**
  * Examples:
  * BioString x = "DEF";
  * BioString y = "ABC";
  * x.prepend(y);
  * // x == "ABCDEF"
  * x = "DEF";
  * x.prepend("BC").prepend("A");
  * // x = "ABCDEF"
  * @param bioString [const BioString &]
  * @return BioString &
  */
inline
BioString &BioString::prepend(const BioString &bioString)
{
    sequence_.prepend(bioString.sequence_);

    return *this;
}

/**
  * @return QString raw BioString sequence data
  */
inline
QString BioString::sequence() const
{
    return sequence_;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static inline methods
/**
  * @param char [const QChar]
  * @return bool
  */
inline
bool BioString::isGap(const QChar &symbol)
{
    return symbol == '-' || symbol == '.';
}

/**
  * Remove any whitespace character (specifically \t \r \n [space]) from sequence and uppercase all characters. Simply
  * converts the QChar to a QString and calls normalize(QString)
  * >> Static function
  * @param sequence [QChar] character of a sequence
  * @return QString
  * @see normalize()
  */
inline
QString BioString::normalize(const QChar &symbol)
{
    return normalize(QString(symbol));
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected inline methods
/**
  * Tests whether the given symbol is a valid biological sequence character (A-Z*.-).
  * @param symbol [const char] a single character code presumably from a biological sequence
  * @return bool
  */
inline
bool BioString::isValid(const char symbol) const
{
    return ((symbol >= 'A' && symbol <= 'Z')
            || symbol == '*'
            || symbol == '-'
            || symbol == '.');
}

/**
  * Tests whether the given symbol is a valid biological sequence character (A-Z*.-).
  * @param symbol [const QChar] a single character code presumably from a biological sequence
  * @return bool
  */
inline
bool BioString::isValid(const QChar &symbol) const
{
    return isValid(symbol.toAscii());
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Addition operator
/**
  * @param left [const BioString &]
  * @param right [const BioString &]
  * @return QString
  */
inline
QString operator+(const BioString &left, const BioString &right)
{
    return left.sequence() + right.sequence();
}

#endif // BIOSTRING_H
