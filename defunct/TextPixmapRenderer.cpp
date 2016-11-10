/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>

#include "TextPixmapRenderer.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param font [const QFont &]
  * @param scale [qreal]
  * @param parent [QObject *]
  *
  * TODO: Find way to test if these signals are connected as expected!
  */
TextPixmapRenderer::TextPixmapRenderer(const QFont &font, qreal scale, QObject *parent) :
    TextImageRenderer(font, scale, parent)
{
    // Disconnect the previous signals for clearing the cache of our parent class, TextImageRenderer
    disconnect(this, SIGNAL(fontChanged()), this, SLOT(clearCache()));
    disconnect(this, SIGNAL(scaleChanged()), this, SLOT(clearCache()));

    // And hook up the signals for clearing this classes cache
    connect(this, SIGNAL(fontChanged()), SLOT(clearCache()));
    connect(this, SIGNAL(scaleChanged()), SLOT(clearCache()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param pointF [const QPointF &]
  * @param character [const char]
  * @param textColorStyle [const TextColorStyle &]
  * @param painter [QPainter *]
  */
void TextPixmapRenderer::drawChar(const QPointF &pointF, const char character, const TextColorStyle &textColorStyle, QPainter *painter) const
{
    ASSERT(painter);

    QString key = cacheKey(character, textColorStyle);
    if (!cachedPixmaps_.contains(key))
        cachedPixmaps_.insert(key, QPixmap::fromImage(renderImage(character, textColorStyle)));

    painter->drawPixmap(pointF, cachedPixmaps_.value(key));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void TextPixmapRenderer::clearCache()
{
    cachedPixmaps_.clear();
}
