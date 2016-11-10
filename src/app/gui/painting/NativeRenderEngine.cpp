/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QBrush>
#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QPen>

#include "NativeRenderEngine.h"
#include "../../graphics/AbstractTextRenderer.h"
#include "../../graphics/TextColorStyle.h"
#include "../../core/macros.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param abstractTextRenderer [AbstractTextRenderer *]
  * @param parent [QObject *]
  */
NativeRenderEngine::NativeRenderEngine(AbstractTextRenderer *abstractTextRenderer, QObject *parent)
    : AbstractRenderEngine(parent),
      abstractTextRenderer_(abstractTextRenderer)
{
    ASSERT(abstractTextRenderer_ != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns AbstractTextRenderer *
  */
AbstractTextRenderer *NativeRenderEngine::abstractTextRenderer() const
{
    return abstractTextRenderer_;
}

/**
  * @param pointF [const QPointF &]
  * @param ch [const char]
  * @param textColorStyle [const TextColorStyle &]
  * @param painter [QPainter *]
  */
void NativeRenderEngine::drawBlockChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter)
{
    ASSERT_X(painter != nullptr, "painter must not be null");

    abstractTextRenderer_->drawChar(pointF, ch, textColorStyle, painter);
}

/**
  * @param p1 [const QPointF &]
  * @param p2 [const QPointF &]
  * @param color [const QColor &]
  * @param painter [QPainter *]
  */
void NativeRenderEngine::drawLine(const QPointF &p1, const QPointF &p2, const QColor &color, QPainter *painter)
{
    ASSERT_X(painter != nullptr, "painter must not be null");

    QPen pen(color, lineWidth_);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(p1, p2);
}

/**
  * Unlike the standard QPainter::drawRect method, which makes the rectangle size equivalent to its width + line width,
  * this method, makes the line fit inside in rect.
  *
  * @param rect [const QRect &]
  * @param color [const QColor &]
  * @param painter [QPainter *]
  */
void NativeRenderEngine::drawRect(const QRect &rect, const QColor &color, QPainter *painter)
{
    ASSERT_X(painter != nullptr, "painter must not be null");

    QPen pen(color, lineWidth_);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
}

/**
  * Unlike the standard QPainter::drawRect method, which makes the rectangle size equivalent to its width + line width,
  * this method, makes the line fit inside in rect.
  *
  * @param rect [const QRectF &]
  * @param color [const QColor &]
  * @param painter [QPainter *]
  */
void NativeRenderEngine::drawRect(const QRectF &rect, const QColor &color, QPainter *painter)
{
    ASSERT_X(painter != nullptr, "painter must not be null");

    QPen pen(color, lineWidth_);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
}

/**
  * @param rect [const QRect &]
  * @param brush [const QBrush &]
  * @param painter [QPainter *]
  */
void NativeRenderEngine::fillRect(const QRect &rect, const QBrush &brush, QPainter *painter)
{
    ASSERT_X(painter != nullptr, "painter must not be null");

    painter->fillRect(rect, brush);
}

/**
  * @param rect [const QRectF &]
  * @param brush [const QBrush &]
  * @param painter [QPainter *]
  */
void NativeRenderEngine::fillRect(const QRectF &rect, const QBrush &brush, QPainter *painter)
{
    ASSERT_X(painter != nullptr, "painter must not be null");

    painter->fillRect(rect, brush);
}

/**
  * Draws an outline inside rect using brush and painter. Lines are guaranteed not to extend outside of rect unless the
  * line width exceeds the rect width or height.
  *
  * @param rect [const QRect &]
  * @param brush [const QBrush &]
  * @param painter [QPainter *]
  */
void NativeRenderEngine::outlineRectInside(const QRect &rect, const QBrush &brush, QPainter *painter)
{
    // Top
    painter->fillRect(rect.left(), rect.top(), rect.width(), lineWidth_, brush);
    // Right (+ 1 is because the right() returns the rightmost pixel)
    painter->fillRect(rect.right() - lineWidth_ + 1, rect.top(), lineWidth_, rect.height(), brush);
    // Bottom (+ 1 is because the bottom() returns the rightmost pixel)
    painter->fillRect(rect.left(), rect.bottom() - lineWidth_ + 1, rect.width(), lineWidth_, brush);
    // Left
    painter->fillRect(rect.left(), rect.top(), lineWidth_, rect.height(), brush);
}

/**
  * Draws an outline for side inside rect using brush and painter.
  *
  * @param rect [const QRect &]
  * @param side [const Side &]
  * @param brush [const QBrush &]
  * @param painter [QPainter *]
  */
void NativeRenderEngine::outlineSideInside(const QRect &rect, const Side &side, const QBrush &brush, QPainter *painter)
{
    switch (side)
    {
    case TopSide:
        painter->fillRect(rect.left(), rect.top(), rect.width(), lineWidth_, brush);
        break;
    case RightSide:
        painter->fillRect(rect.right() - lineWidth_ + 1, rect.top(), lineWidth_, rect.height(), brush);
        break;
    case BottomSide:
        painter->fillRect(rect.left(), rect.bottom() - lineWidth_ + 1, rect.width(), lineWidth_, brush);
        break;
    case LeftSide:
        painter->fillRect(rect.left(), rect.top(), lineWidth_, rect.height(), brush);
        break;
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Reduces the rect size by the line width.
  *
  * @param rect [const QRect &]
  * @returns QRect
  */
QRect NativeRenderEngine::reduceByLineSize(const QRect &rect) const
{
    return QRect(rect.left(),
                 rect.top(),
                 rect.width() - lineWidth_,
                 rect.height() - lineWidth_);
}

/**
  * Reduces the rect size by the line width.
  *
  * @param rect [const QRectF &]
  * @returns QRectF
  */
QRectF NativeRenderEngine::reduceByLineSize(const QRectF &rect) const
{
    return QRectF(rect.left(),
                  rect.top(),
                  rect.width() - lineWidth_,
                  rect.height() - lineWidth_);
}




