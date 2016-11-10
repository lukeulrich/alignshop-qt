/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef AMPLICONRANGE_H
#define AMPLICONRANGE_H

#include <QtCore/QString>

namespace PrimerDesign
{
    /// Represents an amplicon range.
    class AmpliconRange
    {
    public:
        /// The default constructor.
        AmpliconRange();

        /// Initializes with min/max allowable bounds.
        /// @param minBound int
        /// @param maxBound int
        AmpliconRange(int minBound, int maxBound);

        /// Gets the start.
        /// @return int
        int getStart();

        /// Gets the stop.
        /// @return int
        int getStop();

        /// Sets the range.
        /// @param start int
        /// @param stop int
        /// @return bool
        bool setRange(int start, int stop);

    private:
        int start_;
        int stop_;
        int minBound_;
        int maxBound_;
    };
}

#endif // AMPLICONRANGE_H
