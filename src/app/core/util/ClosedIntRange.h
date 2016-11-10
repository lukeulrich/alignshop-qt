/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CLOSEDINTRANGE_H
#define CLOSEDINTRANGE_H

#include <QtCore/QtGlobal>
#include <QtCore/QMetaType>

/**
  * ClosedIntRange models an inclusive integer range using two variables, begin and end.
  *
  * This is a naturally used construct for modeling various spans and in particular when referring to specific regions
  * or segments within a biological sequence. By default, begin and end are set to 0 and -1, respectively, which denotes
  * an empty range.
  *
  * For performance reasons, no value checking is performed and both the begin_ and end_ members are publicly
  * accessible.
  */
class ClosedIntRange
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    /**
      * Construct a closed range spanning from begin to end
      *
      * @param begin [int]
      * @param end [int]
      */
    ClosedIntRange(int begin = 0, int end = -1) : begin_(begin), end_(end)
    {
    }


    // ------------------------------------------------------------------------------------------------
    // Operators
    /**
      * Returns true if the begin and end of other are equivalent to this instance's begin and end; false otherwise
      *
      * @param other [const ClosedIntRange &]
      * @returns bool
      */
    bool operator==(const ClosedIntRange &other) const
    {
        return begin_ == other.begin_ && end_ == other.end_;
    }

    /**
      * Returns true if the begin or end of other are different from this instance's begin and end; false otherwise
      *
      * @param other [const ClosedIntRange &]
      * @returns bool
      */
    bool operator!=(const ClosedIntRange &other) const
    {
        return !operator==(other);
    }

    // Reference for operand names: http://www.factmonster.com/ipka/A0881931.html
    ClosedIntRange &operator+=(const int addend)
    {
        begin_ += addend;
        end_ += addend;

        return *this;
    }

    ClosedIntRange &operator-=(const int subtrahend)
    {
        begin_ -= subtrahend;
        end_ -= subtrahend;

        return *this;
    }

    bool operator<(const ClosedIntRange &other) const
    {
        return begin_ < other.begin_;
    }

    // ------------------------------------------------------------------------------------------------
    // Public methods
    /**
      * @param value [const int &]
      * @returns bool
      */
    bool contains(const int &value) const
    {
        return value >= begin_ && value <= end_;
    }

    /**
      * Returns true if end is less than begin, which denotes a zero-length span
      *
      * @returns bool
      */
    bool isEmpty() const
    {
        return end_ < begin_;
    }

    /**
      * Returns the length spanned from begin to end and is at a minimum zero
      *
      * @returns int
      */
    int length() const
    {
        if (end_ >= begin_)
            return end_ - begin_ + 1;

        return 0;
    }

    /**
      * Swaps the begin and end values
      */
    void invert()
    {
        int tmp = begin_;
        begin_ = end_;
        end_ = tmp;
    }


    // ------------------------------------------------------------------------------------------------
    // Public members
    int begin_;
    int end_;
};

#ifndef QT_NO_DEBUG_STREAM
#include <QtDebug>
QDebug operator<<(QDebug debug, const ClosedIntRange &range);
#endif

Q_DECLARE_TYPEINFO(ClosedIntRange, Q_MOVABLE_TYPE);

Q_DECLARE_METATYPE(ClosedIntRange)

#endif // CLOSEDINTRANGE_H
