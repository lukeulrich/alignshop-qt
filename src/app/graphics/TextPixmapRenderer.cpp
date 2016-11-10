/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>

#include "TextPixmapRenderer.h"
#include "../core/macros.h"

#include "TextColorStyle.h"

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
void TextPixmapRenderer::drawChar(const QPointF &pointF, const char character, const TextColorStyle &textColorStyle, QPainter *painter)
{
    ASSERT(painter);
    ASSERT(character >= 0);
    if (character < 0)
        return;

    QPixmap &pixmap = cachedPixmaps_[static_cast<int>(character)][textColorStyle.foreground_.rgba()][textColorStyle.background_.rgba()];
    if (pixmap.isNull())
        pixmap = QPixmap::fromImage(renderImage(character, textColorStyle));

    painter->drawPixmap(pointF, pixmap);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void TextPixmapRenderer::clearCache()
{
    for (int i=0; i< 128; ++i)
        cachedPixmaps_[i].clear();
}
