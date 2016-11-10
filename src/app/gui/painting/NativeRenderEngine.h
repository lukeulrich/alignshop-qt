/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef NATIVEPAINTENGINE_H
#define NATIVEPAINTENGINE_H

#include "AbstractRenderEngine.h"
#include "../../core/global.h"

class QFont;

class AbstractTextRenderer;

/**
  * Software non-optimized QPainter based engine for basic drawing including block characters.
  */
class NativeRenderEngine : public AbstractRenderEngine
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and desctructor
    NativeRenderEngine(AbstractTextRenderer *abstractTextRenderer, QObject *parent = nullptr);
    virtual AbstractTextRenderer *abstractTextRenderer() const;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual void drawBlockChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter);
    virtual void drawLine(const QPointF &p1, const QPointF &p2, const QColor &color, QPainter *painter);
    virtual void drawRect(const QRect &rect, const QColor &color, QPainter *painter);
    virtual void drawRect(const QRectF &rect, const QColor &color, QPainter *painter);
    virtual void fillRect(const QRect &rect, const QBrush &brush, QPainter *painter);
    virtual void fillRect(const QRectF &rect, const QBrush &brush, QPainter *painter);
    virtual void outlineRectInside(const QRect &rect, const QBrush &brush, QPainter *painter);
    virtual void outlineSideInside(const QRect &rect, const Side &side, const QBrush &brush, QPainter *painter);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    QRect reduceByLineSize(const QRect &rect) const;
    QRectF reduceByLineSize(const QRectF &rect) const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    AbstractTextRenderer *abstractTextRenderer_;
};

#endif // NATIVEPAINTENGINE_H
