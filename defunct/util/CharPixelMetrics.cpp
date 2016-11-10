/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QTextLayout>
#include <QtGui/QTextOption>
#include <cmath>

#include "CharPixelMetrics.h"
#include "Rect.h"
#include "global.h"

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
CharPixelMetrics::CharPixelMetrics(const QFont &font, const qreal scale, const QByteArray &characters) :
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
void CharPixelMetrics::setFont(const QFont &font)
{
    AbstractCharPixelMetrics::setFont(font);
    computeMetrics();
}

/**
  * @param scale [qreal]
  */
void CharPixelMetrics::setScale(qreal scale)
{
    AbstractCharPixelMetrics::setScale(scale);
    computeMetrics();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented protected methods
/**
  * Core method for computing the metrics for all valid characters.
  */
void CharPixelMetrics::computeMetrics()
{
    width_ = 0;
    height_ = 0;

    // --------------------------------------------------------
    // Some variable setup
    QFontMetrics fontMetrics(font());   // To estimate the rendered size of a particular character
    int leftPadding = 2;                // Additional pixel space to pad on the left and right borders
    int rightPadding = 2;
    int maxInkAscent = 0;               // Maximum amount of "ink" extends above and below the baseline
    int maxInkDescent = 0;
    QColor blue = QColor(0, 0, 255);    // Blue is the first bits in the QColor structure and thus the fastest to check
    int baseline = fontMetrics.ascent();
    double scaledBaseline = scale() * baseline;
    int minInkY = scaledBaseline; // Minimum value of space between tallest character and top of image

    // An image buffer for rendering purposes
    QImage buffer(leftPadding + ceil(scale() * fontMetrics.maxWidth()) + rightPadding,
                  ceil(scale() * fontMetrics.height()),
                  QImage::Format_ARGB32_Premultiplied);

    // --------------------------------------------------------
    // Walk through each character in the data and render it, updating our values as necessary and
    // storing in the metrics_ data member.
    QTextLayout layout;
    layout.setFont(font());

    const char *x = characters().constData();
    while (*x)
    {
        // Clear any previous buffer contents
        buffer.fill(0);

        // Render the character
        QPainter painter(&buffer);
        painter.setFont(font());
        painter.setPen(blue);
        painter.translate(leftPadding, scaledBaseline);
        painter.scale(scale(), scale());

        // Using a layout constructed ourselves saves numerous calculations done by theh QPainter::drawText method
        layout.setText(QChar(*x));
        layout.beginLayout();
        layout.createLine();
        layout.endLayout();
        layout.draw(&painter, QPointF(0, -baseline));

        // Accomplishes the same thing as above, but left in case I want to re-use this approach
//        painter.drawText(0., 0., QChar(*x));

        painter.end();

        Rect inkRect = ::boundingRect(buffer);

        // Check for an increase in either the ascent or descent
        if (scaledBaseline - inkRect.top() > maxInkAscent)
            maxInkAscent = scaledBaseline - inkRect.top();
        if (inkRect.bottom() - scaledBaseline > maxInkDescent)
            maxInkDescent = inkRect.bottom() - scaledBaseline;

        // Update the potential maximum width
        if (inkRect.width() > width_)
            width_ = inkRect.width();

        // Update the individual character metric values
        CharMetricF &metric = metrics_[*x - 33];
        metric.inkWidth_ = inkRect.width() + 1;
        metric.inkHeight_ = inkRect.height() + 1;

        metric.inkTopLeft_.ry() = inkRect.top();                         // More calculation to be done in next loop
        if (metric.inkTopLeft_.y() < minInkY)
            minInkY = metric.inkTopLeft_.y();
        // metric.inkTopLeft.x() to be calculated in the next loop

        metric.layoutInkOnlyOrigin_.rx() = -(inkRect.left() - leftPadding);
        metric.layoutInkOnlyOrigin_.ry() = scaledBaseline - inkRect.top();

        ++x;
    }

    // Finish calculating the width and height at this scale
    ++width_;   // Rect returns 1 less than actual number of squares, thus we add one here
    height_ = maxInkAscent + maxInkDescent + 1;     // The additional pixel is for the baseline

    // Having examined each individual character, it is now possible to finalize the remaining
    // class members and the appropriate layout drawing origin and ink top left
    x = characters().constData();
    while (*x)
    {
        CharMetricF &metric = metrics_[*x - 33];
        metric.inkTopLeft_.rx() = (width_ - metric.inkWidth_) / 2.;
        metric.inkTopLeft_.ry() -= minInkY;

        metric.layoutOrigin_.rx() = metric.layoutInkOnlyOrigin_.x() + metric.inkTopLeft_.x();
        metric.layoutOrigin_.ry() = -minInkY;

        ++x;
    }
}
