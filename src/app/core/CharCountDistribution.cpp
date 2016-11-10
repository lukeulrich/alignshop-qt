/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "CharCountDistribution.h"
#include "macros.h"
#include "misc.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param charCounts [const ListHashCharInt &]
  * @param divisor [int]
  */
CharCountDistribution::CharCountDistribution(const VectorHashCharInt &charCounts, int divisor)
    : charCounts_(charCounts), divisor_(divisor)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * @param other [const CharCountDistribution &]
  * @returns bool
  */
bool CharCountDistribution::operator==(const CharCountDistribution &other) const
{
    if (this == &other)
        return true;

    return charCounts_ == other.charCounts_ &&
            divisor_ == other.divisor_;
}

/**
  * @param other [const CharCountDistribution &]
  * @returns bool
  */
bool CharCountDistribution::operator!=(const CharCountDistribution &other) const
{
    return !operator==(other);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Adds the character count values in otherCharCountDistribution to this distribution beginning at offset (1-based).
  * Requirements:
  * o offset must be between 1 and length(), which implies that there must be at least one column
  * o offset + otherCharCountDistribution.length() - 1 must be less than or equal to length()
  *
  * This method does a simple column by column addition of all character counts. If a character count present in
  * otherCharCountDistribution does not exist in this object, transfer that character's count. If an add operation
  * is done without a corresponding subtract operation, it is very possible that the rows value will no longer be
  * valid. It is the user's responsibility to ensure that all operations are properly applied.
  *
  * Example:
  * >> this.charCounts_:           [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
  * >> otherCharCountDistribution: [ (A, 1) (G, 1) ], [ (G, 1) ]
  * >> result:                     [ (A, 3) (C, 2) (G, 1) ], [ (T, 1) (G, 4) ]
  *
  * @param otherCharCountDistribution [const CharCountDistribution &]
  * @param offset [int]
  */
void CharCountDistribution::add(const CharCountDistribution &otherCharCountDistribution, int offset)
{
    ASSERT_X(offset > 0 && offset <= length(), "offset out of range");
    ASSERT_X(offset + otherCharCountDistribution.length() - 1 <= length(), "offset + otherCharCountDistribution - 1 exceeded distribution length");

    const VectorHashCharInt otherCharCounts = otherCharCountDistribution.charCounts();
    for (int i=0, z=otherCharCounts.size(); i<z; ++i)
    {
        const QHash<char, int> &otherHash = otherCharCounts.at(i);
        QHash<char, int> &thisHash = charCounts_[offset + i - 1];

        QHash<char, int>::ConstIterator it;
        for (it = otherHash.constBegin(); it != otherHash.constEnd(); ++it)
            thisHash[it.key()] += it.value();
    }
}

/**
  * @param characters [const QByteArray &]
  * @param skipChar [char]
  * @param offset [int]
  */
void CharCountDistribution::add(const QByteArray &characters, char skipChar, int offset)
{
    ASSERT_X(offset > 0 && offset <= length(), "offset out of range");
    ASSERT_X(offset + characters.length() - 1 <= length(), "offset + characters.length() - 1 exceeded distribution length");

    const char *x = characters.constData();

    // Small optimization
    if (skipChar == '\0')
    {
        for (int i=offset-1; *x; ++i, ++x)
            ++charCounts_[i][*x];
    }
    else // Count all characters regardless
    {
        for (int i=offset-1; *x; ++i, ++x)
            if (*x != skipChar)
                ++charCounts_[i][*x];
    }
}

/**
  * @returns bool
  */
bool CharCountDistribution::allColumnsAreEmpty() const
{
    VectorHashCharInt::ConstIterator it = charCounts_.constBegin();
    for (; it != charCounts_.constEnd(); ++it)
        if ((*it).isEmpty() == false)
            return false;

    return true;
}

/**
  * @returns VectorHashCharInt
  */
VectorHashCharInt CharCountDistribution::charCounts() const
{
    return charCounts_;
}

/**
  * @param range [const ClosedIntRange &]
  * @returns VectorHashCharDouble
  */
VectorHashCharDouble CharCountDistribution::charPercents(const ClosedIntRange &range) const
{
    ASSERT_X(range.isEmpty() || (range.begin_ > 0 && range.begin_ <= length()), "range may not be negative");
    ASSERT_X(range.isEmpty() || (range.end_ > 0 && range.end_ <= length()), "range.end_ out of range");
    ASSERT_X(range.begin_ <= range.end_ || range.isEmpty(), "invalid range");

    if (charCounts_.isEmpty())
        return VectorHashCharDouble();
    else if (range.isEmpty())
        return ::divideVectorHashCharInt(charCounts_, divisor_);
    else
        return ::divideVectorHashCharInt(charCounts_.mid(range.begin_ - 1, range.length()), divisor_);
}

/**
  * @returns int
  */
int CharCountDistribution::divisor() const
{
    return divisor_;
}

/**
  * Inserts count blank entries before position (1-based).
  * Requirements (asserted):
  * o position must be between 1 and length() + 1
  * o count must be greater than or equal to zero
  *
  * Examples:
  * >> insertBlanks(3, 1) -> inserts 1 blank as the third element
  * >> insertBlanks(1, 5) -> inserts 5 blanks at the beginning
  *
  * Given a length of 4, then
  * >> insertBlanks(5, 2) -> inserts 2 blanks at the end
  *
  * @param position [int]
  * @param count [int]
  */
void CharCountDistribution::insertBlanks(int position, int count)
{
    ASSERT_X(position > 0 && position <= length()+1, "position out of range");
    ASSERT(count >= 0);

    charCounts_.insert(position-1, count, HashCharInt());
}

/**
  * @returns int
  */
int CharCountDistribution::length() const
{
    return charCounts_.size();
}

/**
  * @param range [const ClosedIntRange &]
  * @returns CharCountDistribution
  */
CharCountDistribution CharCountDistribution::mid(const ClosedIntRange &range) const
{
    ASSERT(range.isEmpty() == false);
    ASSERT(range.begin_ > 0 && range.begin_ <= range.end_);
    ASSERT(range.end_ <= charCounts_.size());

    return CharCountDistribution(charCounts_.mid(range.begin_ - 1, range.length()), divisor_);
}


/**
  * Removes count entries/columns beginning at position (1-based).
  * Requirements (asserted):
  * o position must be between 1 and length(), which implies that there must be at least one column
  * o count must be greater than or equal to zero
  * o position + count - 1 must be less than or equal to length()
  *
  * Examples:
  * >> remove(3, 1) -> removes the third column
  * >> remove(1, 2) -> inserts the first 2 columns
  *
  * Given a length of 4, then
  * >> remove(3, 2) -> removes the last two columns
  *
  * @param position [int]
  * @param count [int]
  */
void CharCountDistribution::remove(int position, int count)
{
    ASSERT_X(position > 0 && position <= length(), "position out of range");
    ASSERT_X(count >= 0, "count out of range");
    ASSERT_X(position + count - 1 <= length(), "position + count (inclusive) exceeded distribution length");

    charCounts_.remove(position-1, count);
}

/**
  * Removes all character keys from each hash that have a value of zero.
  *
  * Example:
  * >> this.charCounts_:           [ (B, 0) ], [ (A, 2) (C, 2) (G, 0) ], [ (T, 1) (G, 3) ], []
  * >> result:                     [ ], [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ], []
  *
  * @param from [int]
  * @param to [int]
  */
void CharCountDistribution::removeZeroValueKeys(int from, int to)
{
    ASSERT_X(from >= 0 && from <= length(), "from may not be negative");
    ASSERT_X(to >= 0 && to <= length(), "to out of range");
    ASSERT_X(from <= to || to == 0, "from and to out of range");
    ASSERT_X(!(from == 0 && to != 0), "from may not be zero if to is not zero");

    int actualFrom = (from == 0) ? 1 : from;
    int actualTo = (to == 0) ? length() : to;

    for (int i=actualFrom; i<= actualTo; ++i)
    {
         QMutableHashIterator<char, int> it(charCounts_[i-1]);
         while (it.findNext(0))
             it.remove();
    }
}

/**
  * @param divisor [int]
  */
void CharCountDistribution::setDivisor(int divisor)
{
    divisor_ = divisor;
}

/**
  * Subtracts the character count values in otherCharCountDistribution to this distribution beginning at
  * offset (1-based).
  * Requirements:
  * o offset must be between 1 and length(), which implies that there must be at least one column
  * o offset + otherCharCountDistribution.length() - 1 must be less than or equal to length()
  *
  * This method does a simple column by column subtraction of all character counts. If a character count present in
  * otherCharCountDistribution does not exist in this, simply transfer the negative character's count. If a subtract
  * operation is done without a cognate addition operation, it is very possible that the rows value will no longer be
  * valid. It is the user's responsibility to ensure that all operations are properly applied.
  *
  * Example:
  * >> this.charCounts_:           [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
  * >> otherCharCountDistribution: [ (A, 1) (G, 1) ], [ (-, 1) (G, 1) ]
  * >> result:                     [ (A, 2) (C, 2) (G, -2) ], [ (T, 1) (G, 3) (A, -1) (C, -1) ]
  *
  * @param otherCharCountDistribution [const CharCountDistribution &]
  * @param offset [int]
  */
void CharCountDistribution::subtract(const CharCountDistribution &otherCharCountDistribution, int offset)
{
    ASSERT_X(offset > 0 && offset <= length(), "offset out of range");
    ASSERT_X(offset + otherCharCountDistribution.length() - 1 <= length(), "offset + otherCharCountDistribution - 1 exceeded distribution length");

    const VectorHashCharInt otherCharCounts = otherCharCountDistribution.charCounts();
    for (int i=0, z=otherCharCounts.size(); i<z; ++i)
    {
        const QHash<char, int> &otherHash = otherCharCounts.at(i);
        QHash<char, int> &thisHash = charCounts_[offset + i - 1];
        QHash<char, int>::const_iterator it;
        for (it = otherHash.constBegin(); it != otherHash.constEnd(); ++it)
            thisHash[it.key()] -= it.value();
    }
}

/**
  * @param characters [const QByteArray &]
  * @param skipChar [char]
  * @param offset [int]
  */
void CharCountDistribution::subtract(const QByteArray &characters, char skipChar, int offset)
{
    ASSERT_X(offset > 0 && offset <= length(), "offset out of range");
    ASSERT_X(offset + characters.length() - 1 <= length(), "offset + characters.length() - 1 exceeded distribution length");

    const char *x = characters.constData();
    // Small optimization
    if (skipChar == '\0')
    {
        for (int i=offset-1; *x; ++i, ++x)
            --charCounts_[i][*x];
    }
    else
    {
        for (int i=offset-1; *x; ++i, ++x)
            if (*x != skipChar)
                --charCounts_[i][*x];
    }
}
