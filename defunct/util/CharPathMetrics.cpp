/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <cmath>

#include "CharPathMetrics.h"
#include "global.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * All precomputation is performed here and stored for the lifetime of the object.
  *
  * @param font [const QFont &]
  * @param characters [const QByteArray &]
  */
CharPathMetrics::CharPathMetrics(const QFont &font, const QByteArray &characters) : CharPathMetricsF(font, characters)
{
    // Make sure that we only support the appropriate number of characters
    ASSERT(qstrlen(constants::k7BitCharacters) == 94);

    const char *x = characters_.constData();

    qreal extraWidth = (width() - width_) / 2.;
    qreal extraHeight = (height() - height_) / 2.;

    while (*x)
    {
        CharPathMetricF &metric = metrics_[*x - 33];
        metric.origin_.rx() += extraWidth;
        metric.origin_.ry() += extraHeight;

        // Layout should be updated based on extra width
        metric.layoutOriginF_.rx() = metric.origin_.x();
        metric.layoutOriginF_.ry() += extraHeight;

        metric.inkTopLeft_.rx() = (static_cast<double>(width()) - metric.inkSize_.width() ) / 2.;
        metric.inkTopLeft_.ry() += extraHeight;

        ++x;
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int CharPathMetrics::height() const
{
    return static_cast<int>(ceil(height_));
}

/**
  * @param ch [const char]
  * @returns int
  */
int CharPathMetrics::inkHeight(const char ch) const
{
    if (ch < 33 || ch == 127)
        return -1;

    return static_cast<int>(ceil(metrics_[ch-33].inkSize_.height()));
}

/**
  * @param ch [const char]
  * @returns QSize
  */
QSize CharPathMetrics::inkSize(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QSize();

    return QSize(inkWidth(ch), inkHeight(ch));
}

/**
  * @param ch [const char]
  * @returns int
  */
int CharPathMetrics::inkWidth(const char ch) const
{
    if (ch < 33 || ch == 127)
        return -1;

    return static_cast<int>(ceil(metrics_[ch-33].inkSize_.width()));
}

/**
  * @returns QSize
  */
QSize CharPathMetrics::size() const
{
    return QSize(width(), height());
}

/**
  * @returns int
  */
int CharPathMetrics::width() const
{
    return static_cast<int>(ceil(width_));
}
