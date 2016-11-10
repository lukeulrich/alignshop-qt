/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARCOUNTDISTRIBUTION_H
#define CHARCOUNTDISTRIBUTION_H

#include <QtCore/QByteArray>
#include "util/ClosedIntRange.h"
#include "types.h"

/**
  * CharCountDistribution encapsulates the manipulation of a character count distribution which is represented by a
  * ListHashCharInt (see global.h).
  *
  * CharCountDistribution merely wraps a ListHashCharInt and provides useful methods for tweaking its contents. It does
  * not produce any such raw distribution data - this must be supplied upon construction.
  *
  * Specifically, methods are provided for adding and subtracting other character count distributions with respect
  * to this distribution. Additionally, blanks - empty character counts for one or more columns - may be added to the
  * distribution as well as removal of any columns.
  *
  * Note: It is possible to have hash keys with a value of 0. This typically would result from adding or subtracting
  *       another distribution. In essence, this is functionally the same thing as not having this key at all; however,
  *       no care is taken to automatically remove these keys because 1) it requires additional code that carries no
  *       significant benefit and 2) it may be desired in some user cases.
  *
  *       The removeZeroValueKeys method is a convenience method for removing all keys that have a zero value if it is
  *       desired to not have these present.
  */
class CharCountDistribution
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //!< Construct an instance of this class initialized with charCounts and a divisor of 0 (it is user's responsibility to set this properly)
    CharCountDistribution(const VectorHashCharInt &charCounts = VectorHashCharInt(), int divisor = 0);

    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const CharCountDistribution &other) const;                              //!< Returns true if other is equal to this instance; false otherwise
    bool operator!=(const CharCountDistribution &other) const;                              //!< Returns true if other is not equal to this instance; false otherwise

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void add(const CharCountDistribution &otherCharCountDistribution, int offset = 1);      //!< Adds otherCharCountDistribution to this distribution at the specified offset (1-based)
    void add(const QByteArray &characters, char skipChar = '\0', int offset = 1);           //!< Adds all characters except skipChar (if non-zero) beginning at offset to the distribution
    bool allColumnsAreEmpty() const;                                                        //!< Returns true if every column is empty; false otherwise
    VectorHashCharInt charCounts() const;                                                   //!< Returns a copy of the raw character count structure
    //! Transform the character counts spanning range (all columns if range is empty) into percentages and return a vector with this information
    VectorHashCharDouble charPercents(const ClosedIntRange &range = ClosedIntRange()) const;
    int divisor() const;                                                                    //!< Returns the divisor for this distribution
    void insertBlanks(int position, int count);                                             //!< Inserts count blank (empty QHash<char, int>) entries at the given position index (1-based)
    int length() const;                                                                     //!< Returns the length (ie. number of columns)
    CharCountDistribution mid(const ClosedIntRange &range) const;                           //!< Returns a slice of the CharCountDistribution spanning range
    void remove(int position, int count = 1);                                               //!< Removes count entries from the distribution starting at the given position index (1-based)
    void removeZeroValueKeys(int from = 0, int to = 0);                                     //!< Iterates through all values in each column between from and to and removes those keys that have 0 for their value; if both from and to are 0, then analyzes every column; if only from is non-zero, then analyzes all columns of from to length()
    void setDivisor(int divisor);                                                           //!< Sets the divisor value
    void subtract(const CharCountDistribution &otherCharCountDistribution, int offset = 1); //!< Subtracts otherCharCountDistribution from this distribution at the specified offset (1-based)
    void subtract(const QByteArray &characters, char skipChar = '\0', int offset = 1);      //!< Subtracts all characters except skipChar (if non-zero) beginning at offset from the distribution

private:
    VectorHashCharInt charCounts_;
    int divisor_;
};

#endif // CHARCOUNTDISTRIBUTION_H
