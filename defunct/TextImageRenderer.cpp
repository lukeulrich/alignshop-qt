/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "TextImageRenderer.h"

#include <QtGui/QPainter>

#include "global.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * Ensure that the fontChanged and scaleChanged signals clear the cache.
  *
  * @param font [const QFont &]
  * @param scale [qreal]
  * @param parent [QObject *]
  */
TextImageRenderer::TextImageRenderer(const QFont &font, qreal scale, QObject *parent) :
    BasicTextRenderer(font, scale, parent)
{
    connect(this, SIGNAL(fontChanged()), SLOT(clearCache()));
    connect(this, SIGNAL(scaleChanged()), SLOT(clearCache()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  *
  *
  * @param pointF [const QPointF &]
  * @param character [const char]
  * @param textColorStyle [const TextColorStyle &]
  * @param painter [QPainter *]
  */
void TextImageRenderer::drawChar(const QPointF &pointF, const char character, const TextColorStyle &textColorStyle, QPainter *painter) const
{
    ASSERT(painter);

    QString key = cacheKey(character, textColorStyle);
    if (!cachedImages_.contains(key))
        cachedImages_.insert(key, renderImage(character, textColorStyle));

    painter->drawImage(pointF, cachedImages_.value(key));
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Specifically, the unique string is as follows: character + textColorStyle.foreground_.name() +
  * textColorStyle.background_.name().
  *
  * @param character [const character]
  * @param textColorStyle [const TextColorStyle &]
  * @returns QString
  */
QString TextImageRenderer::cacheKey(const char character, const TextColorStyle &textColorStyle) const
{
    return QString(character) + textColorStyle.foreground_.name() + textColorStyle.background_.name();
}

/**
  * Image returned is the ceil'ed
  *
  * @param character [const char]
  * @param textColorStyle [const TextColorStyle &]
  * @returns QImage
  */
QImage TextImageRenderer::renderImage(const char character, const TextColorStyle &textColorStyle) const
{
    QImage image(BasicTextRenderer::size().toSize(), QImage::Format_ARGB32_Premultiplied);

    // Utilize our TextRenderer drawChar method :)
    QPainter painter(&image);

    // No alpha blending!
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    BasicTextRenderer::drawChar(QPointF(0, 0), character, textColorStyle, &painter);
    painter.end();

    return image;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  */
void TextImageRenderer::clearCache()
{
    cachedImages_.clear();
}
