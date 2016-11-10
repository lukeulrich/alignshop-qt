/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCryptographicHash>

#include "BioString.h"
#include "constants.h"
#include "macros.h"
#include "misc.h"

static const char MIN_ASCII_VAL = 32;
static const char MAX_ASCII_VAL = 126;

static int qRegisterTypes()
{
    qRegisterMetaType<BioString>("BioString");
    return 0;
}
Q_CONSTRUCTOR_FUNCTION(qRegisterTypes)

QByteArray BioString::swapByteArray_;   // Static storage for the slide method

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
BioString::BioString() : QByteArray(), grammar_(eUnknownGrammar)
{
}

/**
  * @param grammar [Grammar]
  */
BioString::BioString(Grammar grammar) : QByteArray(), grammar_(grammar)
{
}

/**
  * @param str [const char *]
  * @param grammar [Grammar]
  */
BioString::BioString(const char *str, Grammar grammar) : QByteArray(str), grammar_(grammar)
{
    ::removeWhiteSpace(*this);

#ifdef QT_DEBUG
    checkString(asByteArray(), "BioString::BioString(const char *str, Grammar grammar)");
#endif
}

/**
  * @param byteArray [const QByteArray &]
  * @param grammar [Grammar]
  */
BioString::BioString(const QByteArray &byteArray, Grammar grammar) : QByteArray(byteArray), grammar_(grammar)
{
    ::removeWhiteSpace(*this);

#ifdef QT_DEBUG
    checkString(asByteArray(), "BioString::BioString(const QByteArray &byteArray, Grammar grammar)");
#endif
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param bioString [const BioString &]
  * @returns BioString &
  */
BioString &BioString::operator+=(const BioString &bioString)
{
    ASSERT(grammar_ == bioString.grammar_);

    QByteArray::operator+=(static_cast<QByteArray>(bioString));

    return *this;
}

/**
  * @param byteArray [const QByteArray &]
  * @returns BioString &
  */
BioString &BioString::operator+=(const QByteArray &byteArray)
{
    QByteArray temp(byteArray);
    ::removeWhiteSpace(temp);

#ifdef QT_DEBUG
    checkString(temp.constData(), "BioString::operator+=(const QByteArray &byteArray)");
#endif

    QByteArray::operator+=(temp);

    return *this;
}

/**
  * @param str [const char *]
  * @returns BioString &
  */
BioString &BioString::operator+=(const char *str)
{
    QByteArray byteArray(str);
    ::removeWhiteSpace(byteArray);

#ifdef QT_DEBUG
    checkString(byteArray.constData(), "BioString::operator+=(const char *str)");
#endif

    QByteArray::operator+=(byteArray);

    return *this;
}

/**
  * @param ch [char]
  * @returns BioString &
  */
BioString &BioString::operator+=(char ch)
{
    if (isspace(ch))
        return *this;

    ASSERT(ch >= MIN_ASCII_VAL && ch <= MAX_ASCII_VAL);

    QByteArray::operator+=(ch);

    return *this;
}

/**
  * @param other [const BioString &]
  * @returns bool
  */
bool BioString::operator==(const BioString &other) const
{
    return grammar_ == other.grammar_ && QByteArray::operator==(static_cast<QByteArray>(other));
}

/**
  * @param byteArray [const QByteArray &]
  * @returns bool
  */
bool BioString::operator==(const QByteArray &byteArray) const
{
    return QByteArray::operator==(byteArray);
}
/**
  * @param other [const BioString &]
  * @returns bool
  */
bool BioString::operator==(const char *str) const
{
    return QByteArray::operator==(str);
}

/**
  * @param other [const BioString &]
  * @returns bool
  */
bool BioString::operator!=(const BioString &other) const
{
    return !operator==(other);
}

/**
  * @param byteArray [const QByteArray &]
  * @returns bool
  */
bool BioString::operator!=(const QByteArray &byteArray) const
{
    return !operator==(byteArray);
}

/**
  * @param other [const BioString &]
  * @returns bool
  */
bool BioString::operator!=(const char *str) const
{
    return !operator==(str);
}

/**
  * @param byteArray [const QByteArray &]
  * @returns BioString &
  */
BioString &BioString::operator=(const QByteArray &byteArray)
{
    QByteArray::operator=(byteArray);
    ::removeWhiteSpace(*this);

#ifdef QT_DEBUG
    checkString(asByteArray(), "BioString::operator=(const QByteArray &byteArray)");
#endif

    return *this;
}

/**
  * @param ch [const char &]
  * @returns BioString &
  */
BioString &BioString::operator=(const char &ch)
{
    static char dummy[2] = "X";
    dummy[0] = ch;

    return operator=(dummy);
}

/**
  * @param str [const char *]
  * @returns BioString &
  */
BioString &BioString::operator=(const char *str)
{
    QByteArray::operator=(str);
    ::removeWhiteSpace(*this);

#ifdef QT_DEBUG
    checkString(asByteArray(), "BioString::operator=(const char *str)");
#endif

    return *this;
}



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param bioString [const BioString &]
  * @returns BioString &
  */
BioString &BioString::append(const BioString &bioString)
{
    ASSERT_X(grammar_ == bioString.grammar_, "Incompatible grammar");

    QByteArray::append(static_cast<QByteArray>(bioString));

    return *this;
}

/**
  * @param str [const char *]
  * @returns BioString &
  */
BioString &BioString::append(const char *str)
{
#ifdef QT_DEBUG
    checkString(str, "BioString::append(const char *str)");
#endif

    QByteArray::append(str);

    return *this;
}

/**
  * @param ch [char]
  * @returns BioString &
  */
BioString &BioString::append(char ch)
{
    ASSERT_X(ch >= MIN_ASCII_VAL && ch <= MAX_ASCII_VAL, "ch out of range");

    QByteArray::append(ch);

    return *this;
}

/**
  * @returns QByteArray
  */
QByteArray BioString::asByteArray() const
{
    return static_cast<QByteArray>(*this);
}

/**
  * Will function successfully if the grammar is not RNA; however, it is uncertain when this functionality would be
  * desired.
  *
  * @returns BioString
  */
BioString BioString::backTranscribe() const
{
#ifdef QT_DEBUG
    if (grammar_ != eRnaGrammar)
        qWarning("backTranscribe(): unexpected grammar");
#endif

    BioString dna(*this);
    dna.grammar_ = eDnaGrammar;
    dna.tr("Uu", "Tt");

    return dna;
}

/**
  * Maximally shifts all characters in a given segment to the left.
  *
  * @param range [const ClosedIntRange &]
  * @returns ClosedIntRange
  * @see collapseRight()
  */
ClosedIntRange BioString::collapseLeft(const ClosedIntRange &range)
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    char *charPtr = data();

    // Find first gap
    int firstGap = -1;
    const char *x = charPtr + range.begin_ - 1;
    for (int i=range.begin_; i <= range.end_; ++i)
    {
        if (::isGapCharacter(*x))
        {
            firstGap = i;
            break;
        }
        ++x;
    }

    // Return if the first gap is not found
    ClosedIntRange affectedRange;
    if (firstGap == -1)
        return affectedRange;

    char swapVar;
    char *firstGapPtr = charPtr + firstGap - 1;
    char *y = firstGapPtr + 1; // There is no point in checking that the firstGap character is not a gap character since we already
                                // know this. Furthermore, that is why i is incremented by 1 in the initialization of the following
                                // for loop
    for (int i=firstGap + 1; i <= range.end_; ++i, ++y)
    {
        if (!::isGapCharacter(*y))
        {
            swapVar = *firstGapPtr;
            *firstGapPtr = *y;
            *y = swapVar;

            ++firstGapPtr;

            if (affectedRange.isEmpty())
                affectedRange.begin_ = firstGap;
            affectedRange.end_ = i;
        }
    }

    return affectedRange;
}

/**
  * Maximally shifts all characters in a given segment to the right.
  *
  * @param range [const ClosedIntRange &]
  * @returns ClosedIntRange
  * @see collapseLeft()
  */
ClosedIntRange BioString::collapseRight(const ClosedIntRange &range)
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    char *charPtr = data();

    // Find first gap from the right
    int firstGap = -1;
    const char *x = charPtr + range.end_ - 1;
    for (int i=range.end_; i >= range.begin_; --i)
    {
        if (::isGapCharacter(*x))
        {
            firstGap = i;
            break;
        }
        --x;
    }

    // Return if the first gap is not found
    ClosedIntRange affectedRange;
    if (firstGap == -1)
        return affectedRange;

    char swapVar;
    char *firstGapPtr = charPtr + firstGap - 1;
    char *y = firstGapPtr - 1; // There is no point in checking that the firstGap character is not a gap character since we already
                                // know this. Furthermore, that is why i is decremented by 1 in the initialization of the following
                                // for loop
    for (int i=firstGap - 1; i >= range.begin_; --i, --y)
    {
        if (!::isGapCharacter(*y))
        {
            swapVar = *firstGapPtr;
            *firstGapPtr = *y;
            *y = swapVar;

            --firstGapPtr;

            if (affectedRange.isEmpty())
                affectedRange.end_ = firstGap;
            affectedRange.begin_ = i;
        }
    }

    return affectedRange;
}

/**
  * Uses the following complement rules:
  *
  * Symbol	    A	B	C	D	G	H	K	M	S	T	V	W	N
  * Complement	T	V	G	H	C	D	M	K	S*	A	B	W*	N*
  *
  * Source: http://www.chem.qmul.ac.uk/iubmb/misc/naseq.html
  *
  * @returns BioString
  */
BioString BioString::complement() const
{
    BioString dna(*this, eDnaGrammar);
    dna.tr("ABCDGHKMTVabcdghkmtv", "TVGHCDMKABtvghcdmkab");

    return dna;
}

/**
  * @returns QByteArray
  */
QByteArray BioString::digest() const
{
    return QCryptographicHash::hash(asByteArray(), QCryptographicHash::Md5);
}

/**
  * @param range [const ClosedIntRange &]
  * @returns int
  */
int BioString::gapsBetween(const ClosedIntRange &range) const
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    int nGaps = 0;
    const char *x = constData() + range.begin_ - 1;
    for (int i=range.begin_; i<= range.end_; ++i, ++x)
        if (::isGapCharacter(*x))
            ++nGaps;

    return nGaps;
}

/**
  * Does not consider the sequence character at position.
  *
  * Examples:
  * 12345678
  * AB----CD
  *
  * gapsLeftOf(3) -> 0
  * gapsLeftOf(5) -> 2
  * gapsLeftOf(7) -> 4
  * gapsLeftOf(8) -> 0
  *
  * @param position [int]
  * @returns int
  */
int BioString::gapsLeftOf(int position) const
{
    ASSERT_X(position >= 1 && position <= length(), "position out of range");

    int nGaps = 0;
    // Find correct index in char data:
    // -1 because BioString is 1-based but, the char data is zero-based
    // -1 because we are ignoring the character at position
    const char *x = constData() + position - 1 - 1;
    for (int i=position-1; i >= 1 && ::isGapCharacter(*x); --i, --x)
        ++nGaps;

    return nGaps;
}

/**
  * Does not consider the sequence character at position.
  *
  * Examples:
  * 12345678
  * AB----CD
  *
  * gapsRightOf(1) -> 0
  * gapsRightOf(2) -> 4
  * gapsRightOf(5) -> 1
  * gapsRightOf(6) -> 0
  *
  * @param position [int]
  * @returns int
  */
int BioString::gapsRightOf(int position) const
{
    ASSERT_X(position >= 1 && position <= length(), "position out of range");

    int nGaps = 0;
    // Find correct index in char data:
    // Since BioString is 1-based and char data is zero based, we can simply use the position
    // value to immediately index the character just after the one at position.
    const char *x = constData() + position;
    for (int i=position+1, z=length(); i <= z && ::isGapCharacter(*x); ++i, ++x)
        ++nGaps;

    return nGaps;
}

/**
  * @returns Grammar
  */
Grammar BioString::grammar() const
{
    return grammar_;
}

/**
  * @returns bool
  */
bool BioString::hasGapAt(int position) const
{
    ASSERT_X(position >= 1 && position <= length(), "position out of range");

    const char *x = constData() + position - 1;
    return ::isGapCharacter(*x);
}

/**
  * @returns bool
  */
bool BioString::hasGaps() const
{
    for (const char *x = constData(); *x; ++x)
        if (::isGapCharacter(*x))
            return true;

    return false;
}

/**
  * @returns bool
  */
bool BioString::hasNonGaps() const
{
    for (const char *x = constData(); *x; ++x)
        if (!::isGapCharacter(*x))
            return true;

    return false;
}

/**
  * @returns int
  */
int BioString::headGaps() const
{
    int nGaps = 0;
    for (const char *x = constData(); *x; ++x)
    {
        if (!::isGapCharacter(*x))
            break;

        ++nGaps;
    }

    return nGaps;
}

/**
  * Specifically, the valid range is 1..n+1, where n = length of BioString. Insertion is done at the given index, not
  * after the character at this index.
  *
  * ABCDEF
  * insert(1, "XYZ") -> XYZABCDEF
  * insert(6, "---") -> ABCDE---F
  * insert(7, "..")  -> ABCDEF..
  *
  * @param position [int]
  * @param bioString [const BioString &]
  * @returns BioString &
  */
BioString &BioString::insert(int position, const BioString &bioString)
{
    ASSERT_X(position >= 1 && position <= length() + 1, "position out of range");

    QByteArray::insert(position - 1, static_cast<QByteArray>(bioString));

    return *this;
}

/**
  * The valid range for position is 1..n+1, where n = length of BioString. Insertion is done at the given index, not
  * after the character at this index.
  *
  * Examples:
  * ABCDEF
  * insertGaps(1, 2) -> --ABCDEF
  * insertGaps(3, 0) -> ABCDEF
  *
  * @param position [int]
  * @param nGaps [int]
  * @param gapChar [char]
  * @returns BioString &
  */
BioString &BioString::insertGaps(int position, int nGaps, char gapChar)
{
    ASSERT_X(position >= 1 && position <= length() + 1, "position out of range");
    ASSERT_X(gapChar >= MIN_ASCII_VAL && gapChar <= MAX_ASCII_VAL, "gapChar out of range");
    ASSERT_X(nGaps >= 0, "nGaps must be at least 0");

    if (nGaps < 1)
        return *this;

    QByteArray gap;
    gap.resize(1);
    gap[0] = gapChar;
    QByteArray::insert(position - 1, gap.repeated(nGaps));

    return *this;
}

/**
  * Two BioStrings are essentially equivalent if they are identical with respect to non-gap characters and gap
  * placement.
  *
  * Example:
  * BioString x = "ABC...DEF";
  * x == "ABC---DEF";   // true
  * x == "-ABC...DEF";  // false
  * x == "AB----DEF";   // false
  *
  * @param other [const BioString &]
  * @returns bool
  */
bool BioString::isEquivalentTo(const BioString &other) const
{
    if (grammar_ != other.grammar_)
        return false;

    if (length() != other.length())
        return false;

    const char *x = constData();
    const char *y = other.constData();

    while (*x)
    {
        if (*x == *y || (::isGapCharacter(*x) && ::isGapCharacter(*y)))
        {
            ++x;
            ++y;
            continue;
        }

        return false;
    }

    return true;
}

/**
  * This method differs from indexOf in that the match has to occur beginning at position. The comparison stops at the
  * first non-matching character, rather than continuing to look for other matches. Empty bioString inputs always return
  * false.
  *
  * @param position [int]
  * @param bioString [const BioString &]
  * @returns bool
  */
bool BioString::isExactMatch(int position, const BioString &bioString) const
{
    ASSERT_X(position >= 1 && position <= length(), "position out of range");

#ifdef QT_DEBUG
    if (grammar_ != bioString.grammar_)
        qWarning("[Warning] %s: checking sequence with different grammar", Q_FUNC_INFO);
#endif

    if (bioString.isEmpty())
        return false;

    // Short-cut optimization
    if (position + bioString.length() - 1 > length())
        return false;

    // Compare the sequences character by character
    const char *x = constData() + position - 1;
    const char *y = bioString.constData();
    while (*x && *y)
    {
        if (*x != *y)
            return false;

        ++x;
        ++y;
    }

    return true;
}

/**
  * If gaps are present,
  *
  * @returns bool
  */
bool BioString::isPalindrome() const
{
    return grammar_ == eDnaGrammar &&
           length() > 0 &&
           length() % 2 == 0 &&
           !hasGaps() &&
           *this == reverseComplement();
}

/**
  * @param position [int]
  * @returns bool
  */
bool BioString::isValidPosition(int position) const
{
    return position >= 1 && position <= length();
}

/**
  * @param range [const ClosedIntRange &]
  * @returns bool
  */
bool BioString::isValidRange(const ClosedIntRange &range) const
{
    return !range.isEmpty() &&
           isValidPosition(range.begin_) &&
           isValidPosition(range.end_);
}

/**
  * The number of positions that the characters in range may be slid depends on whether it contains non-gap characters.
  * A segment containing non-gap characters may only be slid the number of gap characters immediately adjacent to the
  * segment on the respective side. If it contains solely gap characters, these may be moved any amount up to the
  * sequence boundaries.
  *
  * Examples:
  *
  * 123456789
  * AB--C-D-E
  *
  * leftSlidablePositions(ClosedIntRange(5, 7)) = 2
  * leftSlidablePositions(ClosedIntRange(6, 6)) = 5
  * leftSlidablePositions(ClosedIntRange(3, 5)) = 0
  *
  * @param range [const ClosedIntRange &]
  * @return int
  * @see rightSlidablePositions(), slide()
  */
int BioString::leftSlidablePositions(const ClosedIntRange &range) const
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    return (nonGapsBetween(range)) ? gapsLeftOf(range.begin_) : range.begin_ - 1;
}

/**
  * Overloaded convenience method for accessing a substring with the same grammar.
  *
  * @param range [const ClosedIntRange &]
  * @returns BioString
  */
BioString BioString::mid(const ClosedIntRange &range) const
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    return BioString(mid(range.begin_, range.length()), grammar_);
}

/**
  * @param range [const ClosedIntRange &]
  * @returns int
  */
int BioString::nonGapsBetween(const ClosedIntRange &range) const
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    int nNonGaps = 0;
    const char *x = constData() + range.begin_ - 1;
    for (int i=range.begin_; i<= range.end_; ++i, ++x)
        if (!::isGapCharacter(*x))
            ++nNonGaps;

    return nNonGaps;
}

/**
  * @returns bool
  */
bool BioString::onlyContainsACGT() const
{
    if (isEmpty())
        return false;

    for (const char *x = constData(); *x; ++x)
    {
        switch (*x)
        {
        case 'A':
        case 'C':
        case 'G':
        case 'T':
            continue;

        default:
            return false;
        }
    }

    return true;
}

/**
  * @param range [const ClosedIntRange &]
  * @returns bool
  */
bool BioString::onlyContainsACGT(const ClosedIntRange &range) const
{
    if (range.isEmpty())
        return false;

    ASSERT_X(isValidRange(range), "Invalid range");

    const char *x = constData() + range.begin_ - 1;
    for (int i=range.begin_; i<= range.end_; ++i, ++x)
    {
        switch (*x)
        {
        case 'A':
        case 'C':
        case 'G':
        case 'T':
            continue;

        default:
            return false;
        }
    }

    return true;
}

/**
  * @param bioString [const BioString &]
  * @returns BioString &
  */
BioString &BioString::prepend(const BioString &bioString)
{
    ASSERT_X(grammar_ == bioString.grammar_, "Incompatible grammar");

    QByteArray::prepend(static_cast<QByteArray>(bioString));

    return *this;
}

/**
  * @param str [const char *]
  * @returns BioString &
  */
BioString &BioString::prepend(const char *str)
{
#ifdef QT_DEBUG
    checkString(str);
#endif

    QByteArray::prepend(str);

    return *this;
}

/**
  * @param ch [char]
  * @returns BioString &
  */
BioString &BioString::prepend(char ch)
{
    ASSERT_X(ch >= MIN_ASCII_VAL && ch <= MAX_ASCII_VAL, "ch out of range");

    QByteArray::prepend(ch);

    return *this;
}

/**
  * @param range [const ClosedIntRange &]
  * @returns BioString &
  */
BioString &BioString::remove(const ClosedIntRange &range)
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    return remove(range.begin_, range.length());
}

/**
  * @returns BioString &
  */
BioString &BioString::removeGaps()
{
    int nNonGaps = 0;
    for (char *x = data(), *y = x; *x; ++x)
    {
        if (!::isGapCharacter(*x))
        {
            *y = *x;
            ++y;
            ++nNonGaps;
        }
    }
    resize(nNonGaps);

    return *this;
}

/**
  * Convenience function for removing one or more contiguous gaps starting at position. Position must reside within the
  * sequence bounds and reference a gap character. Moreover, there must be at least nGaps gaps present beginning at
  * position.
  *
  * @param position [int]
  * @param amount [int]
  * @returns BioString &
  */
BioString &BioString::removeGaps(int position, int nGaps)
{
    ASSERT_X(position >= 1 && position <= length(), "position out of range");
    ASSERT_X(nGaps >= 0, "nGaps must be at least 0");

    /*
    const char *x = constData() + position - 1;
    int nGapsToRemove = 0;
    while (nGapsToRemove < nGaps && ::isGapCharacter(*x))
    {
        ++nGapsToRemove;
        ++x;
    }
    */

    if (nGaps > 0)
    {
        ASSERT(nGaps == gapsBetween(ClosedIntRange(position, position + nGaps - 1)));
        QByteArray::remove(position - 1, nGaps);
    }

    return *this;
}

/**
  * An amount of zero, behaves identically to an insert.
  *
  * @param position [int]
  * @param amount [int]
  * @param bioString [const BioString &]
  * @returns BioString &
  */
BioString &BioString::replace(int position, int amount, const BioString &bioString)
{
    ASSERT_X(position >= 1 && position <= length()+1, "position out of range");
    ASSERT_X(amount >= 0, "amount must be at least zero");
    ASSERT_X(position + amount - 1 <= length(), "amount out of range");

    QByteArray::replace(position - 1, amount, static_cast<QByteArray>(bioString));

    return *this;
}

/**
  * @param range [const ClosedIntRange &]
  * @param bioString [const BioString &]
  * @returns BioString &
  */
BioString &BioString::replace(const ClosedIntRange &range, const BioString &bioString)
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    return replace(range.begin_, range.length(), bioString);
}

/**
  * No explicit test for this since, we are merely calling the QByteArray version of this method.
  *
  * @param before [const BioString &]
  * @param after [const BioString &]
  * @returns BioString &
  */
BioString &BioString::replace(const BioString &before, const BioString &after)
{
    QByteArray::replace(static_cast<QByteArray>(before), static_cast<QByteArray>(after));

    return *this;
}

/**
  * @returns BioString &
  */
BioString &BioString::reverse()
{
    int l = length();
    char *head = data();
    char *tail = head + l - 1;

    char swap;
    for (int i=0, z = l/2; i<z; ++i)
    {
        swap = *head;
        *head = *tail;
        *tail = swap;

        ++head;
        --tail;
    }

    return *this;
}

/**
  * @returns BioString
  */
BioString BioString::reverseComplement() const
{
    BioString dna = complement();
    dna.reverse();
    return dna;
}

/**
  * The number of positions that the characters in range may be slid depends on whether it contains non-gap characters.
  * A segment containing non-gap characters may only be slid the number of gap characters immediately adjacent to the
  * segment on the respective side. If it contains solely gap characters, these may be moved any amount up to the
  * sequence boundaries.
  *
  * Examples:
  *
  * 123456789
  * AB--C-D-E
  *
  * rightSlidablePositions(5, 7) = 1
  * rightSlidablePositions(6, 6) = 3
  * rightSlidablePositions(2, 4) = 0
  *
  * @param range [const ClosedIntRange &]
  * @return int
  * @see leftSlidablePositions(), slide()
  */
int BioString::rightSlidablePositions(const ClosedIntRange &range) const
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    return (nonGapsBetween(range)) ? gapsRightOf(range.end_) : length() - range.end_;
}

/**
  * @param grammar [Grammar]
  */
void BioString::setGrammar(Grammar grammar)
{
    grammar_ = grammar;
}

/**
  * A frequent operation while editing multiple sequence alignments is horizontally sliding a set of characters which
  * "displace" or exchange places with gap positions but not any non-gap character data. Thus, during this method the
  * order of sequence characters is not modified - only the gap positions. The delta parameter specifies the maximum
  * number of positions to horizontally slide. A negative delta indicates to slide the characters to the left and vice
  * versa.
  *
  * While sliding is most often considered in light of moving actual sequence characters, it is also possible
  * to slide a set of gap characters which behave slightly differently. Because swapping positions of a gap does not
  * alter the actual ungapped sequence, it is possibly to move a gap anywhere within the sequence bounds.
  *
  * Example:
  * 1234567890123
  * ABC--D-EF--GH
  *
  * slide(6, 9, -1) -> 1, sequence = ABC-D-EF---GH
  * slide(6, 9, -2) -> 2, sequence = ABCD-EF----GH
  * slide(6, 9, -5) -> same thing as above
  *
  * slide(9, 11, 2) -> 0, unchanged sequence
  * slide(9, 10, 2) -> 1, sequence = ABC--D-E-F-GH
  *
  * NOTE: This method is not thread safe because it utilizes a global static swap byte array.
  *
  * @param range [const ClosedIntRange &]
  * @param delta [int]
  * @returns int
  */
int BioString::slide(const ClosedIntRange &range, int delta)
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    char *swapBuffer = swapByteArray_.data();

    int actualDelta = 0;                  // Stores the distance (in characters) segment was successfully moved
    if (delta < 0)  // Slide to the left
    {
        actualDelta = qMin(-delta, leftSlidablePositions(range));
        if (actualDelta)
        {
            char *source = data() + range.begin_ - 1;
            if (actualDelta > swapByteArray_.size())
            {
                swapByteArray_.resize(actualDelta);
                swapBuffer = swapByteArray_.data();
            }
            char *swap = swapBuffer;

            // A. Get the exact gap representation to the left of the range to be slided
            memcpy(swap, source - actualDelta, actualDelta);

            // B. Move the range to the left
            memmove(source - actualDelta, source, range.length());

            // C. Copy the exact gap representation to the right
            memcpy(source - actualDelta + range.length(), swap, actualDelta);
        }
    }
    else if (delta > 0) // -> Slide to the right
    {
        actualDelta = qMin(delta, rightSlidablePositions(range));
        if (actualDelta)
        {
            char *source = data() + range.begin_ - 1;
            if (actualDelta > swapByteArray_.size())
            {
                swapByteArray_.resize(actualDelta);
                swapBuffer = swapByteArray_.data();
            }
            char *swap = swapBuffer;

            // A. Get the exact gap representation to the right of the range to be slided
            memcpy(swap, source + range.length(), actualDelta);

            // B. Move the range to the right
            memmove(source + actualDelta, source, range.length());

            // C. Copy the exact gap representation to the left
            memcpy(source, swap, actualDelta);
        }
    }

    return actualDelta;
}

/**
  * @returns int
  */
int BioString::tailGaps() const
{
    int nGaps = 0;
    int l = length();
    const char *x = constData() + l - 1;
    while (::isGapCharacter(*x) && l > 0)
    {
        ++nGaps;
        --l;
        --x;
    }

    return nGaps;
}

/**
  * @param before [char]
  * @param after [char]
  * @returns BioString &
  */
BioString &BioString::tr(char before, char after)
{
    ASSERT_X(before >= MIN_ASCII_VAL && before <= MAX_ASCII_VAL, "unallowed before character");
    ASSERT_X(after >= MIN_ASCII_VAL && after <= MAX_ASCII_VAL, "unallowed after character");

    char *x = data();
    while (*x)
    {
        if (*x == before)
            *x = after;

        ++x;
    }

    return *this;
}

/**
  * If the same letter appears multiple times in query, then precedence is given to the lowest index this character
  * occurs at. For example: tr("AaA", "Bb@") will always convert A -> B and ignore the second A -> @ rule.
  *
  * @param query [const char *]
  * @param replacement [const char *]
  * @returns BioString &
  */
BioString &BioString::tr(const char *query, const char *replacement)
{
    ASSERT_X(qstrlen(query) == qstrlen(replacement), "Unequal number of chars in query and replacement");
#ifdef QT_DEBUG
    checkString(query);
    checkString(replacement);
#endif

    int l = qstrlen(query);

    char *x = data();
    while (*x)
    {
        for (int i=0; i<l; ++i)
        {
            if (*x == query[i])
            {
                *x = replacement[i];
                break;  // Only break out of the innermost for loop
            }
        }

        ++x;
    }

    return *this;
}

/**
  * @returns BioString
  */
BioString BioString::transcribe() const
{
#ifdef QT_DEBUG
    if (grammar_ != eDnaGrammar)
        qWarning("%s: unexpected grammar", __FUNCTION__);
#endif

    BioString rna(*this);
    rna.grammar_ = eRnaGrammar;
    rna.tr("Tt", "Uu");

    return rna;
}

/**
  * @param gapChar [char]
  * @returns BioString &
  */
BioString &BioString::translateGaps(char gapChar)
{
    ASSERT_X(gapChar >= MIN_ASCII_VAL && gapChar <= MAX_ASCII_VAL, "gapChar out of range");

    for (char *x = data(); *x; ++x)
        if (::isGapCharacter(*x))
            *x = gapChar;

    return *this;
}

/**
  * @returns BioString
  */
BioString BioString::ungapped() const
{
    BioString result(grammar_);
    result.resize(length());

    char *y = result.data();
    const char *x = constData();
    while (*x)
    {
        if (!::isGapCharacter(*x))
        {
            *y = *x;
            ++y;
        }
        ++x;
    }
    *y = '\0';
    result.resize(qstrlen(result.constData()));

    return result;
}

/**
  * @returns int
  */
int BioString::ungappedLength() const
{
    int nChars = 0;
    for (const char *x = constData(); *x; ++x)
        if (!::isGapCharacter(*x))
            ++nChars;

    return nChars;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public static methods
/**
  * @param byteArray [const QByteArray &]
  * @returns bool
  */
bool BioString::containsUnallowedChars(const QByteArray &byteArray)
{
    for (const char *i = byteArray.constData(); *i; ++i)
        if (*i < MIN_ASCII_VAL || *i > MAX_ASCII_VAL)
            return true;

    return false;
}

/**
  * @param byteArray [const QByteArray &]
  * @param maskChar [char]
  * @returns QByteArray
  */
QByteArray BioString::maskUnallowedChars(const QByteArray &byteArray, char maskChar)
{
    ASSERT_X(maskChar >= MIN_ASCII_VAL && maskChar <= MAX_ASCII_VAL, "unallowed maskChar");

    return maskUnallowedChars(byteArray.constData(), maskChar);
}

/**
  * @param str [const char *]
  * @param maskChar [char]
  * @returns QByteArray
  */
QByteArray BioString::maskUnallowedChars(const char *str, char maskChar)
{
    ASSERT_X(maskChar >= MIN_ASCII_VAL && maskChar <= MAX_ASCII_VAL, "unallowed maskChar");

    QByteArray result(str);
    const char *x = str;
    char *y = result.data();
    while (*x)
    {
        *y = (*x >= MIN_ASCII_VAL && *x <= MAX_ASCII_VAL) ? *x : maskChar;

        ++y;
        ++x;
    }

    return result;
}

/**
  * @param byteArray [const QByteArray &]
  * @returns QByteArray
  */
QByteArray BioString::removeUnallowedChars(const QByteArray &byteArray)
{
    return removeUnallowedChars(byteArray.constData());
}

/**
  * @param str [const char *]
  * @returns QByteArray
  */
QByteArray BioString::removeUnallowedChars(const char *str)
{
    QByteArray result(str);

    int nCharsKept = 0;
    const char *x = str;
    char *y = result.data();
    while (*x)
    {
        if (*x >= MIN_ASCII_VAL && *x <= MAX_ASCII_VAL)
        {
            *y = *x;
            ++y;
            ++nCharsKept;
        }

        ++x;
    }

    result.resize(nCharsKept);

    return result;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
void BioString::checkString(const char *str, const char *location) const
{
#ifdef QT_DEBUG
    const char *i = str;
    while (*i)
    {
        ASSERT_X(*i >= MIN_ASCII_VAL && *i <= MAX_ASCII_VAL, location);
        ++i;
    }
#else
    Q_UNUSED(str);
    Q_UNUSED(location);
#endif
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param range [const ClosedIntRange &]
  * @param delta [int]
  * @returns int
  */
int BioString::slideViaSwap(const ClosedIntRange &range, int delta)
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    int actualDelta = 0;                  // Stores the distance (in characters) segment was successfully moved
    if (delta < 0)  // Slide to the left
    {
        actualDelta = qMin(-delta, leftSlidablePositions(range));
        if (actualDelta)
        {
            bool allGaps = !nonGapsBetween(range);
            if (!allGaps)
                safeSlideLeft(range, actualDelta);
            else
            {
                ClosedIntRange newRange(range.begin_ - actualDelta);
                newRange.end_ = newRange.begin_ + actualDelta - 1;
                safeSlideRight(newRange, range.length());
            }
        }
    }
    else if (delta > 0) // -> Slide to the right
    {
        actualDelta = qMin(delta, rightSlidablePositions(range));
        if (actualDelta)
        {
            bool allGaps = !nonGapsBetween(range);
            if (!allGaps)
                safeSlideRight(range, actualDelta);
            else
            {
                ClosedIntRange newRange(range.end_ + 1);
                newRange.end_ = newRange.begin_ + actualDelta - 1;
                safeSlideLeft(newRange, range.length());
            }
        }
    }

    return actualDelta;
}

/**
  * @param range [const ClosedIntRange &]
  * @param delta [int]
  */
void BioString::safeSlideLeft(const ClosedIntRange &range, int delta)
{
    char *sourceStart = data();
    char *source = sourceStart + range.begin_ - 1;
    char *dest = source - delta;
    for (int i=0, z=range.length(); i< z; ++i, ++source, ++dest)
        qSwap(*source, *dest);
}

/**
  * @param range [const ClosedIntRange &]
  * @param delta [int]
  */
void BioString::safeSlideRight(const ClosedIntRange &range, int delta)
{
    char *sourceStart = data();
    char *source = sourceStart + range.end_ - 1;
    char *dest = source + delta;
    for (int i=0, z= range.length(); i< z; ++i, --source, --dest)
        qSwap(*source, *dest);
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Addition operator
/**
  * @param left [const BioString &]
  * @param right [const BioString &]
  * @return BioString
  */
BioString operator+(const BioString &left, const BioString &right)
{
    ASSERT(left.grammar_ == right.grammar_);

    BioString leftright(left);
    leftright += right;
    return leftright;
}

/**
  * @param left [const BioString &]
  * @param str [const char *]
  * @return BioString
  */
BioString operator+(const BioString &left, const char *str)
{
    BioString result(left);
    result += str;
    return result;
}

/**
  * @param str [const char *]
  * @param right [const BioString &]
  * @return BioString
  */
BioString operator+(const char *str, const BioString &right)
{
    BioString result(str);
    result.grammar_ = right.grammar_;
    result += right;
    return result;
}

/**
  * @param left [const BioString &]
  * @param byteArray [const QByteArray &]
  * @return BioString
  */
BioString operator+(const BioString &left, const QByteArray &byteArray)
{
    BioString result(left);
    result += byteArray;
    return result;
}

/**
  * @param byteArray [const QByteArray &byteArray]
  * @param right [const BioString &]
  * @return BioString
  */
BioString operator+(const QByteArray &byteArray, const BioString &right)
{
    BioString result(byteArray);
    result.grammar_ = right.grammar_;
    result += right;
    return result;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Debugging
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const BioString &bioString)
{
    debug.nospace() << "BioString(";
    debug.nospace();
    switch(bioString.grammar())
    {
    case eUnknownGrammar:
        debug << "Unknown";
        break;
    case eAminoGrammar:
        debug << "Amino";
        break;
    case eDnaGrammar:
        debug << "Dna";
        break;
    case eRnaGrammar:
        debug << "Rna";
        break;
    default:
        debug << "ERROR";
        break;
    }

    debug.nospace() << ", " << bioString.constData() << ")";
    return debug.maybeSpace();
}
#endif
