/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef TEXTIMAGERENDERER_H
#define TEXTIMAGERENDERER_H

#include <QtCore/QHash>
#include <QtGui/QImage>

#include "BasicTextRenderer.h"
#include "TextColorStyle.h"

/**
  * TextImageRenderer caches text renderings with particular color styles as QImages for future painting operations.
  *
  * Storing the result of a rendering operation within a QImage avoids the expensive operational drawing costs.
  */
class TextImageRenderer : public BasicTextRenderer
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Basic constructor initialized with font, scale, and parent
    TextImageRenderer(const QFont &font, qreal scale = 1.0, QObject *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    //! Renders an image of character if not already cached and draws this image at pointF with textColorStyle using the supplied painter
    virtual void drawChar(const QPointF &pointF, const char character, const TextColorStyle &textColorStyle, QPainter *painter) const;

protected:
    //! Returns a unique QString for the character and textColorStyle combination
    QString cacheKey(const char character, const TextColorStyle &textColorStyle) const;
    //! Returns an image sized to fit character with textColorStyle
    QImage renderImage(const char character, const TextColorStyle &textColorStyle) const;

private Q_SLOTS:
    void clearCache();                          //!< Clears all cached images

private:
    mutable QHash<QString, QImage> cachedImages_;

#ifdef TESTING
    friend class TestTextImageRenderer;
#endif
};

#endif // TEXTIMAGERENDERER_H
