/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IRENDERENGINE_H
#define IRENDERENGINE_H

#include <QtCore/QObject>

#include "../../core/global.h"

class QBrush;
class QColor;
class QPainter;
class QPointF;
class QRect;
class QRectF;

class AbstractTextRenderer;
struct TextColorStyle;

enum Side
{
    TopSide = 0,
    RightSide,
    BottomSide,
    LeftSide
};

class IRenderEngine : public QObject
{
public:
    virtual ~IRenderEngine() {}

    virtual AbstractTextRenderer *abstractTextRenderer() const = 0;

    virtual void drawBlockChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter) = 0;
    virtual void drawLine(const QPointF &p1, const QPointF &p2, const QColor &color, QPainter *painter) = 0;
    // Draws outlined rectangle within and up to the rect boundaries. Not outside the boundaries
    virtual void drawRect(const QRect &rect, const QColor &color, QPainter *painter) = 0;
    virtual void drawRect(const QRectF &rect, const QColor &color, QPainter *painter) = 0;
    // Fills the rect without any outline
    virtual void fillRect(const QRect &rect, const QBrush &brush, QPainter *painter) = 0;
    virtual void fillRect(const QRectF &rect, const QBrush &brush, QPainter *painter) = 0;
    virtual void outlineRectInside(const QRect &rect, const QBrush &brush, QPainter *painter) = 0;
    virtual void outlineSideInside(const QRect &rect, const Side &side, const QBrush &brush, QPainter *painter) = 0;

    virtual int lineWidth() const = 0;
    virtual void setLineWidth(int lineWidth) = 0;

protected:
    IRenderEngine(QObject *parent = nullptr) : QObject(parent) {}
};

#endif // IRENDERENGINE_H
