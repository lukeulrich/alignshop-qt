/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef RANGE_H
#define RANGE_H

template<typename T>
class Range
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    /**
      * @param begin [const T &]
      * @param end [const T &]
      */
    Range(const T &begin = 0, const T &end = 0)
        : begin_(begin), end_(end)
    {
    }


    // ------------------------------------------------------------------------------------------------
    // Operators
    /**
      * @param other [const T &]
      * @returns bool
      */
    bool operator==(const T &other) const
    {
        return begin_ == other.begin_ && end_ == other.end_;
    }

    /**
      * @param other [const T &]
      * @returns bool
      */
    bool operator!=(const T &other) const
    {
        return !operator==(other);
    }


    // ------------------------------------------------------------------------------------------------
    // Public methods
    /**
      * @param value [const T &]
      * @returns bool
      */
    bool contains(const T &value) const
    {
        return value >= begin_ && value <= end_;
    }

    /**
      * Returns true if end is less than or equal to begin, which denotes a zero-length span.
      *
      * @returns bool
      */
    bool isEmpty() const
    {
        return end_ <= begin_;
    }

    /**
      * @returns bool
      */
    bool isValid() const
    {
        return begin_ <= end_;
    }

    /**
      * Returns the length spanned from begin to end. May be negative for invalid ranges.
      *
      * @returns int
      */
    int length() const
    {
        return end_ - begin_;
    }


    // ------------------------------------------------------------------------------------------------
    // Public members
    T begin_;
    T end_;
};

typedef Range<double> RangeF;

Q_DECLARE_TYPEINFO(RangeF, Q_MOVABLE_TYPE);

#endif // RANGE_H
