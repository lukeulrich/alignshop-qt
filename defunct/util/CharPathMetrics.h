/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARPATHMETRICS_H
#define CHARPATHMETRICS_H

#include "CharPathMetricsF.h"

class CharPathMetrics : public CharPathMetricsF
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct an instance with the relevant metrics for font and all unique characters; if characters is null, then all valid characters are processed
    CharPathMetrics(const QFont &font, const QByteArray &characters = QByteArray());

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int height() const;                                     //!< Returns the full block height common to all characters
    int inkHeight(const char ch) const;                     //!< Returns the inkable height or "tight" height for ch; or -1 if ch is not valid
    QSize inkSize(const char ch) const;                     //!< Returns the inkable size or "tight" size for ch
    int inkWidth(const char ch) const;                      //!< Returns the inkable width or "tight" width for ch; or -1 if ch is not valid
    QSize size() const;                                     //!< Returns the full block size common to all characters
    int width() const;                                      //!< Returns the full block width common to all characters

#ifdef TESTING
    friend class TestCharPathMetrics;
    friend class TestCharPathMetricsF;      // Uses the inkBoundingRect function; probably should be moved to its own location
#endif
};
#endif // CHARPATHMETRICS_H
