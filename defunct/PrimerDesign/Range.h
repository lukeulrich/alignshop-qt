/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef RANGE_H
#define RANGE_H

namespace PrimerDesign
{
    /// Represents an inclusive range between two values (min and max).
    template <class T>
    class GenericRange
    {
    public:
        // ------------------------------------------------------------------------------------------------
        // Constructors
        /// Constructs a new instance.
        GenericRange()
        {
            min_ = T();
            max_ = T();
        }

        /// Constructs a new instance.
        /// @param min T
        /// @param max T
        GenericRange(T min, T max)
        {
            min_ = min;
            max_ = max;
        }

        // ------------------------------------------------------------------------------------------------
        // Properties
        /// Gets the length.
        /// @return int
        int length() const
        {
            return max_ - min_;
        }

        /// Gets the min value
        /// @return T
        T min() const
        {
            return min_;
        }

        /// Gets the max value
        /// @return T
        T max() const
        {
            return max_;
        }

        // ------------------------------------------------------------------------------------------------
        // Methods
        /// Determines whether or not the current range falls inside of the specified range.
        /// @param range const GenericRange<T> &
        /// @return bool
        bool isInRange(const GenericRange<T> &range) const
        {
            return max_ <= range.max_ && min_ >= range.min_;
        }

        /// Determines if the current range contains the specified value.
        /// @param value T
        /// @return bool
        bool contains(T value) const
        {
            //2011-05-18 (Paul) INDEXING ISSUE HERE : seems that max was set by a subtraction that shortens sequence by 1; had to accomodate this in PrimerPairFinder.cpp line 213
            return value >= min_ && value <= max_;
        }

        // ------------------------------------------------------------------------------------------------
    private:
        T min_;
        T max_;
    };

    /**
      * Represents an integer range.
      */
    typedef GenericRange<int> Range;

    /**
      * Represents a double range.
      */
    typedef GenericRange<double> RangeF;
}

#endif // RANGE_H
