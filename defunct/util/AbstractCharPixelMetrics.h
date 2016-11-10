/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTCHARPIXELMETRICS_H
#define ABSTRACTCHARPIXELMETRICS_H

#include <QtCore/QByteArray>
#include <QtCore/QPointF>
#include <QtCore/QSizeF>
#include <QtGui/QFont>

/**
  * AbstractCharPixelMetrics defines the abstract interface and partial implementation for retrieving scaled information
  * pertaining to a specific font and set of characters.
  *
  * Motivation: Fonts are highly complex with numerous metrics for precisely rendering text in a visually pleasing
  * manner; however, most all font development focuses on producing good results when displayed in paragraph form. There
  * is no good solution for handling individually alignable characters that must be scaled arbitrarily. This partly
  * stems from inadequate Qt support classes and partly from poor font definition files. While monospace fonts in
  * principle solve this problem, the QFontMetrics class for determining the extents and positions of characters is not
  * always ideal. For example, within the generic Monospace font (linux), a capital 'T' contains a left bearing of 1,
  * which at small font sizes, offsets the glyph to far to the right such that it is partially obscured or it leaks over
  * into the next character block. Moreover, it is not possible to determine the tightmost character packing and
  * positioning information with these classes.
  *
  * Currently, only the basic 7-bit ASCII code set is supported, which may be represented by the ascii codes 33-126
  * inclusive.
  */
class AbstractCharPixelMetrics
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Constructs an instance with the relevant metrics for font, scale, and characters; if characters is null, then all valid 7-bit characters are processed
    AbstractCharPixelMetrics(const QFont &font, const qreal scale = 1., const QByteArray &characters = QByteArray());
    virtual ~AbstractCharPixelMetrics();                //!< Trivial destructor

    // ------------------------------------------------------------------------------------------------
    // Public virtual methods
    QByteArray characters() const;
    QFont font() const;
    qreal height() const;                               //!< Returns the full block height common to all characters
    qreal inkHeight(const char ch) const;               //!< Returns the inkable height or "tight" height for ch
    QSizeF inkSize(const char ch) const;                //!< Returns the inkable size or "tight" size for ch
    QPointF inkTopLeft(const char ch) const;            //!< Returns the top left point of the ink-only rectangle relative to width and height
    qreal inkWidth(const char ch) const;                //!< Returns the inkable width or "tight" width for ch; or -1 if ch is not valid
    QPointF layoutInkOnlyOrigin(const char ch) const;   //!< Returns the rendering origin relative to the ink-only portion of ch; in other words, when rendering into a rectangle of inkSize(ch), inkOnlyOrigin returns the drawText point that will render the ink only portion within 0, 0 of the local rectangle coordinates
    QPointF layoutOrigin(const char ch) const;          //!< Returns the rendering origin relative to a default QTextLayout's draw position
    qreal scale() const;                                //!< Returns the current scale factor
    virtual void setFont(const QFont &font);            //!< Sets the font to font
    virtual void setScale(qreal scale);                 //!< Sets the scale to scale; must be greater than 0
    QSizeF size() const;                                //!< Returns the full block size common to all characters
    qreal width() const;                                //!< Returns the full block width common to all characters

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual void computeMetrics() = 0;                  //!< Pure virtual method for computing the necessary metrics

    /**
      * CharMetric simply aggregates the ink width and height, inkTopLeft, layoutOrigin, and layoutInkOnlyOrigin points
      * of interest.
      *
      * All values are defaulted to zero.
      */
    struct CharMetricF
    {
        qreal inkHeight_;
        QPointF inkTopLeft_;
        qreal inkWidth_;
        QPointF layoutInkOnlyOrigin_;
        QPointF layoutOrigin_;

        CharMetricF() : inkHeight_(0), inkWidth_(0)
        {
        }
    };

    qreal width_;                           //!< Scaled full width of a single block
    qreal height_;                          //!< Scaled full height of a single block
    CharMetricF metrics_[94];               //!< Metric space for all 94 characters in 7-bit ASCII

private:
    QFont font_;
    qreal scale_;
    QByteArray characters_;
};

#endif // ABSTRACTCHARPIXELMETRICS_H
