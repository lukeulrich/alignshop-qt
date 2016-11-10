/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "TextImageRenderer.h"

#include <QtGui/QPainter>

#include "TextColorStyle.h"
#include "../core/macros.h"

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
void TextImageRenderer::drawChar(const QPointF &pointF, const char character, const TextColorStyle &textColorStyle, QPainter *painter)
{
    ASSERT(painter);

    QImage &image = cachedImages_[character][textColorStyle.foreground_.rgba()][textColorStyle.background_.rgba()];
    if (image.isNull())
        image = renderImage(character, textColorStyle);

    painter->drawImage(pointF, image);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Image returned is the ceil'ed
  *
  * @param character [const char]
  * @param textColorStyle [const TextColorStyle &]
  * @returns QImage
  */
QImage TextImageRenderer::renderImage(const char character, const TextColorStyle &textColorStyle)
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
