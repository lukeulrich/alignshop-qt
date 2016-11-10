/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARPIXELMETRICSF_H
#define CHARPIXELMETRICSF_H

#include <QtGui/QPainterPath>

#include "AbstractCharPixelMetrics.h"

/**
  * CharPixelMetricsF provides a concrete implementation of AbstractCharPixelMetrics that utilizes a QPainterPath to
  * accurately and precisely compute with floating point precision the specific metrics and rendering positions.
  *
  * The major advantage of this class over CharPixelMetrics is that it provides the precise metrics necessary to render
  * resolution independent graphics of invididually alignable characters. While it would appear that QFontMetricsF would
  * have been the key mechanism for obtaining this information, it turns out that the values returned by QFontMetricsF
  * completely depend on the underlying OS. Currently, only Macintosh systems provide floating point metrics from this
  * class; all others are grid fitted or rounded to integral units.
  *
  * A workaround for obtaining the desired metrics has come through QPainterPath. After adding the character to a
  * QPainterPath, its outlines are mathematically stored and it is possible to obtain the precise width/height and
  * location via the boundingRect method.
  *
  * Determining the exact metrics is a two-step process. First, the unscaled metrics using a QPainterPath are found and
  * second, these are then multiplied by the scale factor. Upon construction and when the font changes, it is necessary
  * to perform both of these steps; however, scaleMetrics is the only calculation required when the scale changes.
  * Because the abstract implementation calls computeMetrics whenever the font or scale changes, this would result in an
  * unnecessary overhead. Thus, the virtual computeMetrics is made into a stub that does nothing and the setScale and
  * setFont virtual methods are overridden to call the appropriate update functions.
  */
class CharPixelMetricsF : public AbstractCharPixelMetrics
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Constructs an instance with the relevant metrics for font, scale, and characters; if characters is null, then all valid 7-bit characters are processed
    CharPixelMetricsF(const QFont &font, const qreal scale = 1., const QByteArray &characters = QByteArray());

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    virtual void setFont(const QFont &font);            //!< Sets the font to font
    virtual void setScale(qreal scale);                 //!< Sets the scale to scale; must be greater than 0

protected:
    // ------------------------------------------------------------------------------------------------
    // Reimplmented protected methods
    virtual void computeMetrics() {}                    //!< Dummy stub to satisfy virtual method; see intro

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void computeUnscaledMetrics();                      //!< Computes the floating point metrics using a QPainterPath
    void scaleMetrics();                                //!< Updates all the scaled metrics by multiplying the unscaled metrics with scale()

    qreal unscaledBlockWidth_;              //!< Unscaled block width
    qreal unscaledBlockHeight_;             //!< Unscaled block height
    qreal unscaledMaxAscent_;               //!< Unscaled negative value indicating the maximum amount that a character ascends above a normal baseline at (0, 0)
    qreal unscaledMaxDescent_;              //!< Unscaled positive value indicating the maximum amount that a character descends below a normal baseline at (0, 0)
    qreal unscaledBaseline_;                //!< Unscaled baseline; relative to top left of block
    CharMetricF unscaledMetrics_[94];       //!< Unscaled metric space for all valid 7-bit characters
};

#endif // CHARPIXELMETRICSF_H
