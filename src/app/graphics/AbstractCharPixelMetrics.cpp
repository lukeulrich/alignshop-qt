/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFontMetrics>

#include "AbstractCharPixelMetrics.h"

#include "../core/constants.h"
#include "../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param font [const QFont &]
  * @param scale [const qreal]
  * @param characters [const QByteArray &]
  */
AbstractCharPixelMetrics::AbstractCharPixelMetrics(const QFont &font, const qreal scale, const QByteArray &characters) :
    blockWidth_(0), blockHeight_(0), font_(font), scale_(scale), characters_(characters)
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
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::baseline() const
{
    return baseline_;
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF AbstractCharPixelMetrics::blockOrigin(const char ch) const
{
    ASSERT(ch >= 33 && ch != 127);

    return metrics_[ch-33].blockOrigin_;
}

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
    return blockHeight_;
}

/**
  * @param ch [const char]
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::inkHeight(const char ch) const
{
    ASSERT(ch >= 33 && ch != 127);

    return metrics_[ch-33].inkSize_.height();
}

/**
  * @param ch [const char]
  * @returns QSizeF
  */
QSizeF AbstractCharPixelMetrics::inkSize(const char ch) const
{
    ASSERT(ch >= 33 && ch != 127);

    const CharMetricF &metric = metrics_[ch - 33];
    return metric.inkSize_;
}

/**
  * @param ch [const char]
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::inkWidth(const char ch) const
{
    ASSERT(ch >= 33 && ch != 127);

    return metrics_[ch-33].inkSize_.width();
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF AbstractCharPixelMetrics::layoutInkOnlyOrigin(const char ch) const
{
    ASSERT(ch >= 33 && ch != 127);

    return metrics_[ch-33].layoutInkOnlyOrigin_;
}

/**
  * @param ch [const char]
  * @returns QPointF
  */
QPointF AbstractCharPixelMetrics::layoutOrigin(const char ch) const
{
    ASSERT(ch >= 33 && ch != 127);

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
    computeMetrics();
}

/**
  * @param scale [qreal]
  */
void AbstractCharPixelMetrics::setScale(qreal scale)
{
    ASSERT(scale > 0);

    scale_ = scale;
    computeMetrics();
}

/**
  * @returns QSizeF
  */
QSizeF AbstractCharPixelMetrics::size() const
{
    return QSizeF(blockWidth_, blockHeight_);
}

/**
  * @returns qreal
  */
qreal AbstractCharPixelMetrics::width() const
{
    return blockWidth_;
}

int AbstractCharPixelMetrics::maxWidth(const QFontMetrics &fontMetrics) const
{
#ifdef Q_OS_MAC
    int maxWidth = 0;
    for (const char *x = characters_.constData(); *x; ++x)
    {
        int charWidth = fontMetrics.width(*x);
        if (charWidth > maxWidth)
            maxWidth = charWidth;
    }
    return maxWidth;
#else
    return fontMetrics.maxWidth();
#endif
}

qreal AbstractCharPixelMetrics::maxWidth(const QFontMetricsF &fontMetricsF) const
{
#ifdef Q_OS_MAC
    int maxWidth = 0;
    for (const char *x = characters_.constData(); *x; ++x)
    {
        int charWidth = fontMetricsF.width(*x);
        if (charWidth > maxWidth)
            maxWidth = charWidth;
    }
    return maxWidth;
#else
    return fontMetricsF.maxWidth();
#endif
}
