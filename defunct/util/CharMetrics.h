/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARMETRICS_H
#define CHARMETRICS_H

#include <QtCore/QByteArray>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#include "Rect.h"

/**
  * CharMetrics computes the specific metrics and rendering positions for a set of characters belonging to a given font
  * such that it is possible to render alignable characters within a static block size.
  *
  * Motivation: Fonts are highly complex with numerous metrics for precisely rendering text in a visually pleasing
  * manner; however, most all font development focuses on producing good results when displayed in paragraph form. There
  * is no good solution for handling individually alignable characters that must be scaled arbitrarily. This partly
  * stems from an inadequate Qt support classes and partly from poor font definition files. While monospace fonts in
  * principle solve this problem, the QFontMetrics class for determining the extents and positions of characters is not
  * always ideal. For example, within the generic Monospace font (linux), a capital 'T' contains a left bearing of 1,
  * which at small font sizes, offsets the glyph to far to the right such that it is partially obscured or it leaks over
  * into the next character block. Thus, CharMetrics was implemented to reliably determine the character positions when
  * rendered and thus accurately render individual characters in a truly alignable manner.
  *
  * The major approach taken here is to create an oversized image buffer and then render each character into this buffer
  * and measure it's bounding rect of rendered characters. Offsets for each character are stored such that it is
  * possible when calling QPainter methods to precisely render the character at a specific location. The width and
  * height define the outermost block size that will fully contain all specific characters. Moreover, two sets of
  * offsets are computed. The first, inkOnlyOrigin, is for purely rendering only the "inkable" portion of the character.
  * The second, layoutOrigin, is for properly rendering the character relative to the block size centered horizontally
  * and vertically relative to the common baseline.
  *
  * Currently, only the basic 7-bit ASCII code set is supported, which may be represented by the ascii codes 33-126
  * inclusive.
  *
  * By default all metrics are set to a default of -1, thus if a user requests the data for a character outside the
  * normal 33-126 range or one that has not been specified via the characters argument to the constructor, they will
  * receive a null value (QPoint) or -1 (int).
  *
  * A floating point version of this class, CharMetricsF, provides for exactly positioning glyphs relative to the design
  * metrics included in the font.
  *
  * @see CharMetricsF
  */
class CharMetrics
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct an instance with the relevant metrics for font and all unique characters; if characters is null, then all valid characters are processed
    CharMetrics(const QFont &font, const QByteArray &characters = QByteArray());

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int height() const;                                     //!< Returns the full block height common to all characters
    int inkHeight(const char ch) const;                     //!< Returns the inkable height or "tight" height for ch; or -1 if ch is not valid
    QPoint inkOnlyOrigin(const char ch) const;              //!< Returns the rendering origin relative to the ink-only portion of ch; in other words, when rendering into a rectangle of inkSize(ch), inkOnlyOrigin returns the drawText point that will render the ink only portion within 0, 0 of the local rectangle coordinates
    QSize inkSize(const char ch) const;                     //!< Returns the inkable size or "tight" size for ch
    QPoint inkTopLeft(const char ch) const;                 //!< Returns the top left point of the ink-only rectangle relative to width and height
    int inkWidth(const char ch) const;                      //!< Returns the inkable width or "tight" width for ch; or -1 if ch is not valid
    QPoint layoutOrigin(const char ch) const;               //!< Returns the rendering origin relative to a default QTextLayout's draw position
    QPoint origin(const char ch) const;                     //!< Returns the rendering origin relative to the character width and height
    QSize size() const;                                     //!< Returns the full block size common to all characters
    int width() const;                                      //!< Returns the full block width common to all characters

private:
    /**
      * CharMetric simply aggregates the origin, inkOrigin, layoutOrigin, and inkTopLeft points of interest
      */
    struct CharMetric
    {
        QPoint inkOnlyOrigin_;  //!< Origin to be used when rendering via QPainter::drawText such that only the ink portion is rendered relative to inkSize
        QSize inkSize_;         //!< Size of the ink based portion
        QPoint inkTopLeft_;     //!< Offset relative to size() for the ink portion
        QPoint layoutOrigin_;   //!< Origin to use when rendering via QTextLayout::draw
        QPoint origin_;         //!< Origin to be used when rendering via QPainter::drawText such that that ink porition is centered relative to size()
    };

    int height_;                //!< Full block width
    int width_;                 //!< Full block width
    CharMetric metrics_[94];    //!< Metrics for all 94 characters in 7-bit ASCII

#ifdef TESTING
    friend class TestCharMetrics;
    friend class TestCharMetricsF;      // Uses the inkBoundingRect function; probably should be moved to its own location
#endif
};

#endif // CHARMETRICS_H
