/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "MockMsaView.h"
#include "util/Rect.h"
#include "TextPixmapRenderer.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QWidget *]
  */
MockMsaView::MockMsaView(QWidget *parent) : AbstractMsaView(parent)
{
    abstractTextRenderer_ = new TextPixmapRenderer(font(), zoom());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param painter [QPainter *]
  */
void MockMsaView::drawAll(QPainter *painter)
{
    drawCalls_ << DrawCall("drawAll");
    AbstractMsaView::drawAll(painter);
}

/**
  * @param painter [QPainter *]
  */
void MockMsaView::drawBackground(QPainter * /* painter */) const
{
    drawCalls_ << DrawCall("drawBackground");
}

/**
  * @param origin [const QPointF &]
  * @param msaRect [const QRect &]
  * @param painter [QPainter *]
  */
void MockMsaView::drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter * /* painter */)
{
    drawCalls_ << DrawCall("drawMsa");
    drawCalls_.last().arguments_ << origin << qVariantFromValue(msaRect);
}

/**
  * @param rect [const QRectF &]
  * @param painter [QPainter *]
  */
void MockMsaView::drawSelection(const QRectF &rect, QPainter * /* painter */) const
{
    drawCalls_ << DrawCall("drawSelection");
    drawCalls_.last().arguments_ << rect;
}

/**
  * @param rect [const QRectF &]
  * @param painter [QPainter *]
  */
void MockMsaView::drawEditCursor(const QRectF &rect, QPainter * /* painter */) const
{
    drawCalls_ << DrawCall("drawEditCursor");
    drawCalls_.last().arguments_ << rect;
}

/**
  * @param rect [const QRectF &]
  * @param painter [QPainter *]
  */
void MockMsaView::drawMouseActivePoint(const QRectF &rect, QPainter * /* painter */) const
{
    drawCalls_ << DrawCall("drawMouseActivePoint");
    drawCalls_.last().arguments_ << rect;
}

/**
  * @param x [qreal]
  * @param painter [QPainter *]
  */
void MockMsaView::drawGapInsertionLine(qreal x, QPainter *painter) const
{
    Q_UNUSED(x);
    Q_UNUSED(painter);
}

/**
  * @param newMsaRegionClip [const Rect &]
  */
void MockMsaView::setMsaRegionClip(const Rect &newMsaRegionClip)
{
    AbstractMsaView::setMsaRegionClip(newMsaRegionClip);

    drawCalls_ << DrawCall("setMsaRegionClip");
    drawCalls_.last().arguments_ << qVariantFromValue(newMsaRegionClip);
}

AbstractTextRenderer *MockMsaView::abstractTextRenderer() const
{
    return abstractTextRenderer_;
}
