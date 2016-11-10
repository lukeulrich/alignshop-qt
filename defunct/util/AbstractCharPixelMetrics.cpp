/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AbstractCharPixelMetrics.h"

#include "global.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param font [const QFont &]
  * @param scale [const qreal]
  * @param characters [const QByteArray &]
  */
AbstractCharPixelMetrics::AbstractCharPixelMetrics(const QFont &font, const qreal scale, const QByteArray &characters) :
    width_(0), height_(0), font_(font), scale_(scale), characters_(characters)
{
    ASSERT(scale > 0.);

    if (characters_.isEmpty())
        characters_ = constants::k7BitCharacters;
}

/**
  */
AbstractCharPixelMetrics::~AbstractCharPixelMetrics()
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QByteArray
  */
QByteArray AbstractCharPixelMetrics::characters() const
{
    return characters_;
}

/**
  * @returns QFont
  */
QFont AbstractCharPixelMetrics::font() const
{
    return font_;
}

/**
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::height() const
{
    return height_;
}

/**
  * @param ch [const char]
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::inkHeight(const char ch) const
{
    ASSERT(ch > 33 && ch != 127);

    return metrics_[ch-33].inkHeight_;
}

/**
  * @param ch [const char]
  * @returns QSizeF
  */
QSizeF AbstractCharPixelMetrics::inkSize(const char ch) const
{
    ASSERT(ch > 33 && ch != 127);

    const CharMetricF &metric = metrics_[ch - 33];
    return QSizeF(metric.inkWidth_, metric.inkHeight_);
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF AbstractCharPixelMetrics::inkTopLeft(const char ch) const
{
    ASSERT(ch > 33 && ch != 127);

    return metrics_[ch-33].inkTopLeft_;
}

/**
  * @param ch [const char]
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::inkWidth(const char ch) const
{
    ASSERT(ch > 33 && ch != 127);

    return metrics_[ch-33].inkWidth_;
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF AbstractCharPixelMetrics::layoutInkOnlyOrigin(const char ch) const
{
    ASSERT(ch > 33 && ch != 127);

    return metrics_[ch-33].layoutInkOnlyOrigin_;
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF AbstractCharPixelMetrics::layoutOrigin(const char ch) const
{
    ASSERT(ch > 33 && ch != 127);

    return metrics_[ch-33].layoutOrigin_;
}

/**
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::scale() const
{
    return scale_;
}

/**
  * @param font [const QFont &]
  */
void AbstractCharPixelMetrics::setFont(const QFont &font)
{
    font_ = font;
}

/**
  * @param scale [qreal]
  */
void AbstractCharPixelMetrics::setScale(qreal scale)
{
    ASSERT(scale > 0);

    scale_ = scale;
}

/**
  * @returns QSizeF
  */
QSizeF AbstractCharPixelMetrics::size() const
{
    return QSizeF(width_, height_);
}

/**
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::width() const
{
    return width_;
}
