/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFontMetrics>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "CharMetrics.h"
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
CharMetrics::CharMetrics(const QFont &font, const QByteArray &characters) : height_(-1), width_(-1)
{
    // Make sure that we only support the appropriate number of characters
    ASSERT(qstrlen(constants::k7BitCharacters) == 94);

    // --------------------------------------------------------
    // Some variable setup
    QFontMetrics fontMetrics(font);     // To estimate the rendered size of a particular character
    int leftPadding = 2;                // Additional pixel space to pad on the left and right borders
    int rightPadding = 2;
    int maxInkAscent = 0;               // Maximum amount of "ink" extends above and below the baseline
    int maxInkDescent = 0;
    QColor blue = QColor(0, 0, 255);    // Blue is the first bits in the QColor structure and thus the fastest to check
    int minInkY = fontMetrics.ascent(); // Minimum value of space between tallest character and top of image

    // And of course an image buffer for rendering purposes
    QImage buffer(leftPadding + fontMetrics.maxWidth() + rightPadding,
                  fontMetrics.height(),
                  QImage::Format_ARGB32_Premultiplied);

    // If characters is empty, perform metrics for all possible 7-bit characters
    QByteArray actualCharacters = characters;
    if (characters.isEmpty())
        actualCharacters = constants::k7BitCharacters;

    // --------------------------------------------------------
    // Walk through each character in the data and render it, updating our values as necessary and
    // storing in the metrics_ data member.
    const char *x = actualCharacters.data();
    while (*x)
    {
        // Clear any previous buffer contents
        buffer.fill(0);

        // Render the character
        QPainter painter(&buffer);
        painter.setFont(font);
        painter.setPen(blue);
        painter.drawText(leftPadding, fontMetrics.ascent(), QChar(*x));
        painter.end();

        Rect inkRect = ::boundingRect(buffer);

        // Check for an increase in either the ascent or descent
        if (fontMetrics.ascent() - inkRect.top() > maxInkAscent)
            maxInkAscent = fontMetrics.ascent() - inkRect.top();
        if (inkRect.bottom() - fontMetrics.ascent() > maxInkDescent)
            maxInkDescent = inkRect.bottom() - fontMetrics.ascent();

        // Update the potential maximum width
        if (inkRect.width() + 1 > width_)
            width_ = inkRect.width() + 1;

        // Update the individual character metric values
        CharMetric &metric = metrics_[*x - 33];
        metric.inkTopLeft_.ry() = inkRect.top();
        metric.inkOnlyOrigin_.rx() = -(inkRect.left() - leftPadding);
        metric.inkOnlyOrigin_.ry() = fontMetrics.ascent() - inkRect.top();      // Or inkTopLeft.y()
        metric.inkSize_ = QSize(inkRect.width() + 1,
                                inkRect.height() + 1);
        if (metric.inkTopLeft_.y() < minInkY)
            minInkY = metric.inkTopLeft_.y();

        ++x;
    }

    // Having examined each individual character, it is now possible to finalize the remaining
    // class members and the appropriate painting origin
    x = actualCharacters.data();
    while (*x)
    {
        CharMetric &metric = metrics_[*x - 33];
        metric.origin_.rx() = metric.inkOnlyOrigin_.x() + ( (width_ - metric.inkSize_.width()) / 2 );
        metric.origin_.ry() = fontMetrics.ascent() - minInkY;

        metric.inkTopLeft_.rx() = ( (width_ - metric.inkSize_.width()) / 2 );
        metric.inkTopLeft_.ry() -= minInkY;

        metric.layoutOrigin_.rx() = metric.origin_.x();
        metric.layoutOrigin_.ry() = -minInkY;

        ++x;
    }

    height_ = maxInkAscent + maxInkDescent + 1;     // The additional pixel is for the baseline
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int CharMetrics::height() const
{
    return height_;
}

/**
  * @param ch [const char]
  * @returns int
  */
int CharMetrics::inkHeight(const char ch) const
{
    if (ch < 33 || ch == 127)
        return -1;

    return metrics_[ch-33].inkSize_.height();
}

/**
  * @param ch [const char]
  * @returns QPoint
  */
QPoint CharMetrics::inkOnlyOrigin(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QPoint();

    return metrics_[ch-33].inkOnlyOrigin_;
}

/**
  * @param ch [const char]
  * @returns QSize
  */
QSize CharMetrics::inkSize(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QSize();

    return metrics_[ch-33].inkSize_;
}

/**
  * @param ch [const char]
  * @returns QPoint
  */
QPoint CharMetrics::inkTopLeft(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QPoint();

    return metrics_[ch-33].inkTopLeft_;
}

/**
  * @param ch [const char]
  * @returns int
  */
int CharMetrics::inkWidth(const char ch) const
{
    if (ch < 33 || ch == 127)
        return -1;

    return metrics_[ch-33].inkSize_.width();
}

/**
  * @param ch [const char]
  * @returns QPoint
  */
QPoint CharMetrics::layoutOrigin(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QPoint();

    return metrics_[ch-33].layoutOrigin_;
}

/**
  * @param ch [const char]
  * @returns QPoint
  */
QPoint CharMetrics::origin(const char ch) const
{
    if (ch < 33 || ch == 127)
        return QPoint();

    return metrics_[ch-33].origin_;
}

/**
  * @returns QSize
  */
QSize CharMetrics::size() const
{
    return QSize(width_, height_);
}

/**
  * @returns int
  */
int CharMetrics::width() const
{
    return width_;
}
