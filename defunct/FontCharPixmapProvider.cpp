/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "FontCharPixmapProvider.h"

#include <QtGui/QPainter>

#include "math.h"

#include "global.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * Scale must be greater than or equal to zero.
  *
  * @param font [const QFont &]
  * @param scale [qreal]
  * @param parent [QObject *]
  */
FontCharPixmapProvider::FontCharPixmapProvider(const QFont &font, qreal scale, QObject *parent) :
    AbstractCharPixmapProvider(parent), font_(font), scale_(scale), fontMetricsF_(font)
{
    ASSERT(scale_ >= 0);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QFont
  */
QFont FontCharPixmapProvider::font() const
{
    return font_;
}

/**
  * @returns qreal
  */
qreal FontCharPixmapProvider::height() const
{
    return scale_ * fontMetricsF_.height();
}

/**
  * @returns qreal
  */
qreal FontCharPixmapProvider::scale() const
{
    return scale_;
}

/**
  * @returns qreal
  */
qreal FontCharPixmapProvider::unscaledHeight() const
{
    return fontMetricsF_.height();
}

/**
  * @param character [char]
  * @returns qreal
  */
qreal FontCharPixmapProvider::unscaledWidth(char character) const
{
    return fontMetricsF_.width(character);
}

/**
  * @param character [char]
  * @returns qreal
  */
qreal FontCharPixmapProvider::width(char character) const
{
    return scale_ * fontMetricsF_.width(character);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * If the font is changed, set the font, recalculate its metrics, and then clear the cache.
  *
  * @param font [const QFont &]
  */
void FontCharPixmapProvider::setFont(const QFont &font)
{
    if (font == font_)
        return;

    font_ = font;
    fontMetricsF_ = QFontMetricsF(font_);

    clearCache();
    emit fontChanged();
}

/**
  * Scale must be greater than zero. If the scale is changed, then the cache is cleared.
  *
  * @param scale [qreal]
  */
void FontCharPixmapProvider::setScale(qreal scale)
{
    ASSERT(scale > 0);

    if (qFuzzyCompare(scale, scale_))
        return;

    scale_ = scale;

    clearCache();
    emit scaleChanged();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Core method responsible for rendering the glyph pixmap for the character and color combination.
  *
  * @param character [char]
  * @param color [const QColor &]
  * @returns QPixmap
  */
QPixmap FontCharPixmapProvider::renderGlyph(char character, const QColor &color)
{
    // If the scale is zero, then simply return a 0x0 pixmap
    if (qFuzzyCompare(scale_, 0))
        return QPixmap(0, 0);

    // Draw the scaled character on an image surface
    QImage image(QSize(static_cast<int>(width(character)), static_cast<int>(height())), QImage::Format_ARGB32_Premultiplied);
    image.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&image);
    painter.setFont(font_);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setPen(color);
    painter.scale(scale_, scale_);

    // Drawing with unscaled parameters because it will scale automatically
//	painter.drawText(fontMetricsF_.leftBearing(character), fontMetricsF_.ascent(), QChar(character));
	painter.drawText(QPointF(0., fontMetricsF_.ascent()), QChar(character));
	painter.end();

	return QPixmap::fromImage(image);
}
