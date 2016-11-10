/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARMETRICSF_H
#define CHARMETRICSF_H

#include <QtCore/QByteArray>
#include <QtCore/QPointF>
#include <QtCore/QSizeF>
#include <QtGui/QFont>

/**
  * CharMetricsF computes with floating point precision the specific metrics and rendering positions for a set of
  * characters belonging to a given font such that it is possible to render alignable characters within a static block
  * size.
  *
  * The major advantage of this class over CharMetrics is that it provides the precise metrics necessary to render
  * resolution independent graphics of invididually alignable characters. While it would appear that QFontMetricsF would
  * have been the key mechanism for obtaining this information, it turns out that the values returned by QFontMetricsF
  * completely depend on the underlying OS. Currently, only Macintosh systems provide floating point metrics from this
  * class; all others are grid fitted or rounded to integral units.
  *
  * A workaround for obtaining the desired metrics has come through QPainterPath. After adding the character to a
  * QPainterPath, it's outlines are mathematically stored and it is possible to obtain its precise width/height and
  * location via the boundingRect method. Thus, all functionaliy of this class follows that of the CharMetrics class,
  * yet, the results are obtained slightly differently and reflect the design metrics contained in the source font.
  *
  * Due to the floating point nature and complicated rendering process, it is very difficult to adequately test this
  * class. Therefore, until a more brilliant idea comes along, testing will be skipped for this class.
  *
  * @see CharMetrics
  */
class CharMetricsF
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct an instance with the relevant metrics for font and all unique characters; if characters is null, then all valid characters are processed
    CharMetricsF(const QFont &font, const QByteArray &characters = QByteArray());

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

    // Deprecated
    //    QPointF layoutOrigin(const char ch) const;
private:
    /**
      * CharMetricF simply aggregates the origin, inkOrigin, and inkTopLeft points of interest
      */
    struct CharMetricF
    {
        QPointF inkOnlyOrigin_;     //!< Origin to be used when rendering via QPainter::drawText such that only the ink portion is rendered relative to inkSize
        QSizeF inkSize_;            //!< Size of the ink based portion
        QPointF inkTopLeft_;        //!< Offset relative to size() for the ink portion
        QPointF layoutOriginF_;     //!< Origin to use when rendering via QTextLayout::draw
        QPointF origin_;            //!< Origin to be used when rendering via QPainter::drawText such that that ink porition is centered relative to size()

        // Deprecated
        //    QPointF layoutOrigin_;
    };

    qreal height_;                //!< Full block width
    qreal width_;                 //!< Full block width
    CharMetricF metrics_[94];    //!< Metrics for all 94 characters in 7-bit ASCII

#ifdef TESTING
    friend class TestCharMetricsF;
#endif
};

#endif // CHARMETRICSF_H
