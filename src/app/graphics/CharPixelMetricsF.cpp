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
    computeUnscaledMetrics();
    scaleMetrics();
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
    computeUnscaledMetrics();
    scaleMetrics();
}

/**
  * @param scale [qreal]
  */
void CharPixelMetricsF::setScale(qreal scale)
{
    AbstractCharPixelMetrics::setScale(scale);
    scaleMetrics();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Core method for computing the metrics for all valid characters. Only updates the unscaled metrics.
  *
  * @see scaleMetrics()
  */
void CharPixelMetricsF::computeUnscaledMetrics()
{
    unscaledBlockWidth_ = 0;
    unscaledBlockHeight_ = 0;

    // --------------------------------------------------------
    // Some variable setup
    unscaledMaxAscent_ = 0;
    unscaledMaxDescent_ = 0;

    // --------------------------------------------------------
    // Walk through each character in the data and render it, updating our values as necessary and
    // storing in the metrics_ data member.
    for (const char *x = characters().constData(); *x; ++x)
    {
        QPainterPath path;
        path.addText(0, 0, font(), QChar(*x));
        QRectF inkRect = path.boundingRect();

        // Update the individual character metric values
        CharMetricF &metric = unscaledMetrics_[*x - 33];
        metric.bearing_.rx() = inkRect.left();
        metric.bearing_.ry() = inkRect.top();
        metric.xMin_ = inkRect.left();
        metric.xMax_ = inkRect.right();
        metric.yMin_ = inkRect.top();
        metric.yMax_ = inkRect.bottom();
        metric.inkSize_ = inkRect.size();

        // Update the maximum width
        if (inkRect.width() > unscaledBlockWidth_)
            unscaledBlockWidth_ = inkRect.width();

        // Check for an increase in either the ascent or descent
        if (metric.yMin_ < unscaledMaxAscent_)
            unscaledMaxAscent_ = metric.yMin_;
        if (metric.yMax_ > unscaledMaxDescent_)
            unscaledMaxDescent_ = metric.yMax_;

        metric.layoutInkOnlyOrigin_.rx() = -metric.xMin_;
        metric.layoutInkOnlyOrigin_.ry() = -metric.yMin_;
    }

    // Baseline is in positive units relative to the top left of the block
    unscaledBaseline_ = -unscaledMaxAscent_;
    unscaledBlockHeight_ = unscaledBaseline_ + unscaledMaxDescent_;

    QFontMetricsF fontMetrics(font());
    qreal fontAscent = fontMetrics.ascent();
    qreal minInkY = fontAscent - unscaledMaxAscent_;

    // Having examined each individual character, it is now possible to finalize the remaining
    // class members and the appropriate painting origin
    for (const char *x = characters().constData(); *x; ++x)
    {
        CharMetricF &metric = unscaledMetrics_[*x - 33];
        metric.blockOrigin_.rx() = ((unscaledBlockWidth_ - metric.inkSize_.width()) / 2.) - metric.bearing_.x();
        metric.blockOrigin_.ry() = unscaledBaseline_;

        metric.layoutOrigin_.rx() = metric.blockOrigin_.rx();
        metric.layoutOrigin_.ry() = -minInkY;
    }

    // Compute the scaled heights and widths
    scaleMetrics();
}


/**
  * Updates all the actual metrics reported by AbstractCharPixelMetrics with their appopriately scaled values.
  */
void CharPixelMetricsF::scaleMetrics()
{
    qreal scale = AbstractCharPixelMetrics::scale();

    blockWidth_ = unscaledBlockWidth_ * scale;
    blockHeight_ = unscaledBlockHeight_ * scale;
    baseline_ = unscaledBaseline_ * scale;

    for (const char *x = characters().constData(); *x; ++x)
    {
        CharMetricF &unscaledMetric = unscaledMetrics_[*x - 33];
        CharMetricF &metric = metrics_[*x - 33];

        metric.bearing_ = unscaledMetric.bearing_ * scale;
        metric.xMin_ = unscaledMetric.xMin_ * scale;
        metric.xMax_ = unscaledMetric.xMax_ * scale;
        metric.yMin_ = unscaledMetric.yMin_ * scale;
        metric.yMax_ = unscaledMetric.yMax_ * scale;
        metric.inkSize_ = unscaledMetric.inkSize_ * scale;
        metric.blockOrigin_ = unscaledMetric.blockOrigin_ * scale;
        metric.layoutOrigin_ = unscaledMetric.layoutOrigin_ * scale;
        metric.layoutInkOnlyOrigin_ = unscaledMetric.layoutInkOnlyOrigin_ * scale;
    }
}
