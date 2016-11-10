/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARPIXELMETRICS_H
#define CHARPIXELMETRICS_H

#include "AbstractCharPixelMetrics.h"

/**
  * CharPixelMetrics provides a concrete implementation of AbstractCharPixelMetrics that utilizes an image buffer to
  * exactly compute the positioning information of each glyph.
  *
  * The major approach taken here is to create an oversized image buffer and then render each character into this buffer
  * and measure it's bounding rect of rendered characters. Offsets for each character are stored such that it is
  * possible when calling QPainter methods to precisely render the character at a specific location. The width and
  * height define the outermost block size that will fully contain all specific characters.
  *
  * The setFont() and setScale() methods are overridden to update the metrics in response to these changes.
  */
class CharPixelMetrics : public AbstractCharPixelMetrics
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Constructs an instance with the relevant metrics for font, scale, and characters; if characters is null, then all valid 7-bit characters are processed
    CharPixelMetrics(const QFont &font, const qreal scale = 1., const QByteArray &characters = QByteArray());

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    virtual void setFont(const QFont &font);        //!< Sets the font to font
    virtual void setScale(qreal scale);             //!< Sets the scale to scale; must be greater than 0

protected:
    // ------------------------------------------------------------------------------------------------
    // Reimplmented protected methods
    virtual void computeMetrics();                  //!< Called whenever the necessary metrics need to be recomputed
};


#endif // CHARPIXELMETRICS_H
