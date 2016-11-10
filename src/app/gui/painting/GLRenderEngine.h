/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GLPAINTENGINE_H
#define GLPAINTENGINE_H

#include "AbstractRenderEngine.h"

class TextImageRenderer;

class GLPaintEngine : public AbstractRenderEngine
{
public:
    GLPaintEngine(const QFont &font, QObject *parent = nullptr);
    virtual AbstractTextRenderer *abstractTextRenderer() const;

    virtual void drawBlockChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter);
    virtual void drawLine(const QPointF &p1, const QPointF &p2, const QColor &color, QPainter *painter);
    virtual void drawRect(const QRectF &rect, const QColor &color, QPainter *painter);
    virtual void fillRect(const QRectF &rect, const QColor &color, QPainter *painter);

private Q_SLOTS:
    // Connect to these and update the GL textures
    void onFontChanged();
    void onScaleChanged();

private:
    TextImageRenderer *textPixmapRenderer_;
};

#endif // GLPAINTENGINE_H
