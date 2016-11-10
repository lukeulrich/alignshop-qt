/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef TEXTPIXMAPRENDERER_H
#define TEXTPIXMAPRENDERER_H

#include "TextImageRenderer.h"

#include <QtGui/QPixmap>

#include "global.h"

/**
  * TextPixmapRenderer extends TextImageRenderer by caching QPixmap representations instead of QImages.
  *
  */
class TextPixmapRenderer : public TextImageRenderer
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Basic constructor initialized with font, scale, and parent
    explicit TextPixmapRenderer(const QFont &font, qreal scale = 1.0, QObject *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    //! Renders a pixmap of character if not already cached and draws this pixmap at pointF with textColorStyle using the supplied painter
    virtual void drawChar(const QPointF &pointF, const char character, const TextColorStyle &textColorStyle, QPainter *painter) const;

private Q_SLOTS:
    void clearCache();                          //!< Clears all cached pixmaps

private:
    mutable QHash<QString, QPixmap> cachedPixmaps_;

#ifdef TESTING
    friend class TestTextPixmapRenderer;
#endif
};

#endif // TEXTPIXMAPRENDERER_H
