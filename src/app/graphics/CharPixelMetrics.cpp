/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <cmath>

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QTextLayout>
#include <QtGui/QTextOption>

#include "CharPixelMetrics.h"
#include "../core/util/Rect.h"
#include "graphics_misc.h"

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
// Reimplemented protected methods
/**
  * The baseline is a zero-based value. All non-descending characters will fill pixels up to but not including the
  * baseline. For example, the letter A does not descend below the baseline and thus when rendered at a baseline of
  * 12, its bottommost pixel will be located at the 11th zero-based index. On the other hand, any character that
  * descends will occupy the baseline row.
  *
  * In the calcluations below, the maxInkAscent does not include the baseline, but the maxInkDescent does.
  *
  * Core method for computing the metrics for all valid characters.
  */
void CharPixelMetrics::computeMetrics()
{
    blockWidth_ = 0;
    blockHeight_ = 0;

    // --------------------------------------------------------
    // Some variable setup
    QFontMetrics fontMetrics(font());   // To estimate the rendered size of a particular character
    int leftPadding = 2;                // Additional pixel space to pad on the left and right borders
    int rightPadding = 2;
    int maxAscent = 0;                  // Maximum amount of "ink" extends above the baseline (negative value)
    int maxDescent = 0;                 // Maximum amount of "ink" extends below the baseline (negative value)
    QColor blue = QColor(0, 0, 255);    // Blue occupies the first 8 bits in the QColor structure (and thus should be the fastest to check?)
    int baseline = fontMetrics.ascent();        // 1-based
    double scaledBaseline = scale() * baseline;
    int minInkY = scaledBaseline;      // Minimum value of space between tallest character and top of image

//    qDebug() << "Baseline:" << baseline;
//    qDebug() << "Scaled baseline:" << scaledBaseline;
//    qDebug() << "Max font width:" << fontMetrics.maxWidth();
//    qDebug() << "Scaled max font width:" << ceil(scale() * fontMetrics.maxWidth());

    // An image buffer for rendering purposes
    QImage buffer(leftPadding + ceil(scale() * maxWidth(fontMetrics)) + rightPadding,
                  ceil(scale() * fontMetrics.height()),
                  QImage::Format_ARGB32_Premultiplied);

    // --------------------------------------------------------
    // Walk through each character in the data and render it, updating our values as necessary and
    // storing in the metrics_ data member.
    QTextLayout layout;
    layout.setFont(font());

    for (const char *x = characters().constData(); *x; ++x)
    {
        // Clear any previous buffer contents
        buffer.fill(0);

        // Render the character
        QPainter painter(&buffer);
        painter.setFont(font());
        painter.translate(leftPadding, scaledBaseline);
        painter.scale(scale(), scale());

        // Draws the baseline a greenish color (debug purposes)
//        painter.setPen(QColor(0, 255, 0, 96));
//        painter.drawLine(0, 0, buffer.width()-1, 0);

        // Using a layout constructed ourselves saves numerous calculations done by the QPainter::drawText method
        layout.setText(QChar(*x));
        layout.beginLayout();
        layout.createLine();
        layout.endLayout();
        painter.setPen(blue);
        layout.draw(&painter, QPointF(0, -baseline));

        // Accomplishes the same thing as above, but left in case I want to re-use this approach
//        painter.drawText(0., 0., QChar(*x));

        painter.end();

        Rect inkRect = ::boundingRect(buffer);

        // Update the individual character metric values
        CharMetricF &metric = metrics_[*x - 33];
        metric.bearing_.rx() = inkRect.left() - leftPadding;
        metric.bearing_.ry() = inkRect.top() - scaledBaseline;
        metric.xMin_ = inkRect.left() - leftPadding;
        metric.xMax_ = inkRect.right() - leftPadding;
        metric.yMin_ = inkRect.top() - scaledBaseline;
        metric.yMax_ = inkRect.bottom() - scaledBaseline;
        metric.inkSize_ = inkRect.size();

        // Update the maximum width
        if (inkRect.width() > blockWidth_)
            blockWidth_ = inkRect.width();

        // Check for an increase in either the ascent or descent
        if (metric.yMin_ < maxAscent)
            maxAscent = metric.yMin_;
        if (metric.yMax_ > maxDescent)
            maxDescent = metric.yMax_;

        if (inkRect.top() < minInkY)
            minInkY = inkRect.top();

        metric.layoutInkOnlyOrigin_.rx() = -metric.xMin_;
        metric.layoutInkOnlyOrigin_.ry() = scaledBaseline - inkRect.top();
    }

    // Finish calculating the width and height at this scale
    ++blockWidth_;   // Rect returns 1 less than actual number of squares, thus we add one here
    baseline_ = -maxAscent;
    blockHeight_ = baseline_ + maxDescent;

//    qDebug() << "BlockWidth" << blockWidth_;
//    qDebug() << "Baseline" << baseline_;
//    qDebug() << "BlockHeight" << blockHeight_;
//    qDebug() << "MinInkY:" << minInkY;

    // Having examined each individual character, it is now possible to finalize the remaining
    // class members and the appropriate layout drawing origin and ink top left
    for (const char *x = characters().constData(); *x; ++x)
    {
        CharMetricF &metric = metrics_[*x - 33];
        metric.blockOrigin_.rx() = ((blockWidth_ - metric.inkSize_.width()) / 2.) - metric.bearing_.x();
        metric.blockOrigin_.ry() = baseline_;

        metric.layoutOrigin_.rx() = metric.blockOrigin_.rx();
        metric.layoutOrigin_.ry() = -minInkY;
    }
}
