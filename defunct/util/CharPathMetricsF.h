/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARPATHMETRICSF_H
#define CHARPATHMETRICSF_H

#include <QtCore/QByteArray>
#include <QtCore/QPointF>
#include <QtCore/QSizeF>

#include <QtGui/QFont>

class CharPathMetricsF
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct an instance with the relevant metrics for font and all unique characters; if characters is null, then all valid characters are processed
    CharPathMetricsF(const QFont &font, const QByteArray &characters = QByteArray());

    // ------------------------------------------------------------------------------------------------
    // Public methods
    qreal height() const;                                   //!< Returns the full block height common to all characters
    qreal inkHeight(const char ch) const;                   //!< Returns the inkable height or "tight" height for ch; or -1 if ch is not valid
    QPointF inkOnlyOrigin(const char ch) const;             //!< Returns the rendering origin relative to the ink-only portion of ch; in other words, when rendering into a rectangle of inkSize(ch), inkOnlyOrigin returns the point that will render the ink only portion within 0, 0 of the local rectangle coordinates
    QSizeF inkSize(const char ch) const;                    //!< Returns the inkable size or "tight" size for ch
    QPointF inkTopLeft(const char ch) const;                //!< Returns the top left point of the ink-only rectangle relative to width and height
    qreal inkWidth(const char ch) const;                    //!< Returns the inkable width or "tight" width for ch; or -1 if ch is not valid
    QPointF layoutOrigin(const char ch) const;              //!< Returns the rendering origin relative to a default QTextLayout
    QPointF origin(const char ch) const;                    //!< Returns the rendering origin relative to the character width and height
    QSizeF size() const;                                    //!< Returns the full block size common to all characters
    qreal width() const;                                    //!< Returns the full block width common to all characters

protected:
    /**
      * CharMetricF simply aggregates the origin, inkOrigin, and inkTopLeft points of interest
      */
    struct CharPathMetricF
    {
        QPointF inkOnlyOrigin_;     //!< Origin to be used when rendering via QPainter::drawText such that only the ink portion is rendered relative to inkSize
        QSizeF inkSize_;            //!< Size of the ink based portion
        QPointF inkTopLeft_;        //!< Offset relative to size() for the ink portion
        QPointF layoutOriginF_;     //!< Origin to use when rendering via QTextLayout::draw
        QPointF origin_;            //!< Origin to be used when rendering via QPainter::drawText such that that ink porition is centered relative to size()
    };

    qreal height_;                //!< Full block width
    qreal width_;                 //!< Full block width
    CharPathMetricF metrics_[94]; //!< Metrics for all 94 characters in 7-bit ASCII
    QByteArray characters_;

#ifdef TESTING
    friend class TestCharMetricsF;
#endif
};
#endif // CHARPATHMETRICSF_H
