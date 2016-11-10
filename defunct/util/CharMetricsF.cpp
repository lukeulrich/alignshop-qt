/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "CharMetricsF.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainterPath>

#include "global.h"

#include <cmath>

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param font [const QFont &]
  * @param characters [const QByteArray &]
  */
CharMetricsF::CharMetricsF(const QFont &font, const QByteArray &characters) : height_(0), width_(0)
{
    // Make sure that we only support the appropraiate number of characters
    ASSERT(qstrlen(constants::k7BitCharacters) == 94);

    // --------------------------------------------------------
    // Some variable setup
    qreal maxInkAscent = 0;               // Maximum the amount of "ink" extends above and below the baseline
    qreal maxInkDescent = 0;

    // If characters is empty, perform metrics for all possible 7-bit characters
    QByteArray actualCharacters = characters;
    if (characters.isEmpty())
        actualCharacters = constants::k7BitCharacters;

    // --------------------------------------------------------
    // Walk through each character in the data and render it, updating our values as necessary and
    // storing in the metrics_ data member.
    const char *x = actualCharacters.constData();
    while (*x)
    {
        QPainterPath path;
        path.addText(0, 0, font, QChar(*x));
        QRectF inkRect = path.boundingRect();

        // Check for an increase in either the ascent or descent
        if (-inkRect.top() > maxInkAscent)
            maxInkAscent = -inkRect.top();
        if (inkRect.bottom() > maxInkDescent)
            maxInkDescent = inkRect.bottom();

        // Update the potential maximum width
        if (inkRect.width() > width_)
            width_ = inkRect.width();

        // Update the individual character metric values
        CharMetricF &metric = metrics_[*x - 33];
        metric.inkOnlyOrigin_.rx() = -inkRect.left();
        metric.inkOnlyOrigin_.ry() = -inkRect.top();
        metric.inkSize_ = inkRect.size();

        metric.inkTopLeft_.ry() = -inkRect.top();

        ++x;
    }

    height_ = maxInkAscent + maxInkDescent;

    // Having examined each individual character, it is now possible to finalize the remaining
    // class members and the appropriate painting origin
    QFontMetrics fm(font);
    x = actualCharacters.constData();
    while (*x)
    {
        CharMetricF &metric = metrics_[*x - 33];
        metric.origin_.rx() = metric.inkOnlyOrigin_.x() + ( (width_ - metric.inkSize_.width()) / 2. );
        metric.origin_.ry() = maxInkAscent;

        metric.layoutOriginF_.rx() = metric.origin_.x();
        metric.layoutOriginF_.ry() = -(fm.ascent() - metric.origin_.y());

//        metric.layoutOrigin_.rx() = metric.inkOnlyOrigin_.x() + ( (iWidth - metric.inkSize_.width()) / 2. );
//        metric.layoutOrigin_.ry() = -(fm.ascent() - metric.origin_.y());

        metric.inkTopLeft_.rx() = ( (width_ - metric.inkSize_.width()) / 2. );
        metric.inkTopLeft_.ry() = maxInkAscent - metric.inkTopLeft_.y();

        ++x;
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns qreal
  */
qreal CharMetricsF::height() const
{
    return height_;
}

/**
  * @param ch [const char]
  * @returns qreal
  */
qreal CharMetricsF::inkHeight(const char ch) const
{
    if (ch < 33 || ch == 127)
        return -1.;

    return metrics_[ch-33].inkSize_.height();
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF CharMetricsF::inkOnlyOrigin(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QPointF();

    return metrics_[ch-33].inkOnlyOrigin_;
}

/**
  * @param ch [const char]
  * @returns QSizeF
  */
QSizeF CharMetricsF::inkSize(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QSizeF();

    return metrics_[ch-33].inkSize_;
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF CharMetricsF::inkTopLeft(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QPointF();

    return metrics_[ch-33].inkTopLeft_;
}

/**
  * @param ch [const char]
  * @returns qreal
  */
qreal CharMetricsF::inkWidth(const char ch) const
{
    if (ch < 33 || ch == 127)
        return -1.;

    return metrics_[ch-33].inkSize_.width();
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
//QPointF CharMetricsF::layoutOrigin(const char ch) const
//{
//    if (ch < 33 || ch == 127)
//        return QPointF();
//
//    return metrics_[ch-33].layoutOrigin_;
//}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF CharMetricsF::layoutOrigin(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QPointF();

    return metrics_[ch-33].layoutOriginF_;
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF CharMetricsF::origin(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QPointF();

    return metrics_[ch-33].origin_;
}

/**
  * @returns QSizeF
  */
QSizeF CharMetricsF::size() const
{
    return QSizeF(width_, height_);
}

/**
  * @returns qreal
  */
qreal CharMetricsF::width() const
{
    return width_;
}
