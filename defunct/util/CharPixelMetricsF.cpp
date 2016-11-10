/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFontMetricsF>

#include "CharPixelMetricsF.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * Computes the metrics upon instantiation.
  *
  * @param font [const QFont &]
  * @param scale [const qreal]
  * @param characters [const QByteArray &]
  */
CharPixelMetricsF::CharPixelMetricsF(const QFont &font, const qreal scale, const QByteArray &characters) :
    AbstractCharPixelMetrics(font, scale, characters)
{
    computeMetrics();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * @param font [const QFont &]
  */
void CharPixelMetricsF::setFont(const QFont &font)
{
    AbstractCharPixelMetrics::setFont(font);
    computeMetrics();
}

/**
  * @param scale [qreal]
  */
void CharPixelMetricsF::setScale(qreal scale)
{
    AbstractCharPixelMetrics::setScale(scale);
    updateMetrics();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented protected methods
/**
  * Core method for computing the metrics for all valid characters. Only updates the unscaled metrics.
  *
  * @see updateMetrics()
  */
void CharPixelMetricsF::computeMetrics()
{
    unscaledWidth_ = 0;
    unscaledHeight_ = 0;

    // --------------------------------------------------------
    // Some variable setup
    qreal maxInkAscent = 0;               // Maximum the amount of "ink" extends above and below the baseline
    qreal maxInkDescent = 0;

    QFontMetricsF fontMetrics(font());
    qreal fontAscent = fontMetrics.ascent();
    qreal minInkY = fontAscent;

    // --------------------------------------------------------
    // Walk through each character in the data and render it, updating our values as necessary and
    // storing in the metrics_ data member.
    const char *x = characters().constData();
    while (*x)
    {
        QPainterPath path;
        path.addText(0, 0, font(), QChar(*x));
        QRectF inkRect = path.boundingRect();

        qreal charAscent = -inkRect.top();
        qreal charDescent = inkRect.bottom();

        // Check for an increase in either the ascent or descent
        if (charAscent > maxInkAscent)
            maxInkAscent = charAscent;
        if (charDescent > maxInkDescent)
            maxInkDescent = charDescent;

        if (fontAscent - charAscent < minInkY)
            minInkY = fontAscent - charAscent;

        // Update the potential maximum width
        if (inkRect.width() > unscaledWidth_)
            unscaledWidth_ = inkRect.width();

        // Update the individual character metric values
        CharMetricF &metric = unscaledMetrics_[*x - 33];
        metric.inkWidth_ = inkRect.width();
        metric.inkHeight_ = inkRect.height();

        metric.inkTopLeft_.ry() = fontAscent - charAscent;

        metric.layoutInkOnlyOrigin_.rx() = -inkRect.left();
        metric.layoutInkOnlyOrigin_.ry() = charAscent;

        ++x;
    }

    unscaledHeight_ = maxInkAscent + maxInkDescent;

    // Having examined each individual character, it is now possible to finalize the remaining
    // class members and the appropriate painting origin
    x = characters().constData();
    while (*x)
    {
        CharMetricF &metric = unscaledMetrics_[*x - 33];
        metric.inkTopLeft_.rx() = (unscaledWidth_ - metric.inkWidth_) / 2.;
        metric.inkTopLeft_.ry() = maxInkAscent - metric.inkTopLeft_.y();

        metric.layoutOrigin_.rx() = metric.layoutInkOnlyOrigin_.x() + metric.inkTopLeft_.x();
        metric.layoutOrigin_.ry() = -minInkY;

        ++x;
    }

    // Compute the scaled heights and widths
    updateMetrics();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Updates all the actual metrics reported by AbstractCharPixelMetrics with their appopriately scaled values.
  */
void CharPixelMetricsF::updateMetrics()
{
    qreal scale = AbstractCharPixelMetrics::scale();

    width_ = unscaledWidth_ * scale;
    height_ = unscaledHeight_ * scale;

    const char *x = characters().constData();
    while (*x)
    {
        CharMetricF &unscaledMetric = unscaledMetrics_[*x - 33];
        CharMetricF &metric = metrics_[*x - 33];

        metric.inkWidth_ = unscaledMetric.inkWidth_ * scale;
        metric.inkHeight_ = unscaledMetric.inkHeight_ * scale;

        metric.inkTopLeft_ = unscaledMetric.inkTopLeft_ * scale;
        metric.layoutInkOnlyOrigin_ = unscaledMetric.layoutInkOnlyOrigin_ * scale;
        metric.layoutOrigin_ = unscaledMetric.layoutOrigin_ * scale;

        ++x;
    }
}
