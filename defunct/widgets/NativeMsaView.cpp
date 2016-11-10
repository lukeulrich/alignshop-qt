/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "NativeMsaView.h"

#include <QtGui/QPainter>

#include <cmath>

#include "../Msa.h"
#include "../PositionalMsaColorProvider.h"
#include "../TextPixmapRenderer.h"


#include "global.h"

#include <QtSvg/QSvgGenerator>
#include <QtGui/QFontMetricsF>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
NativeMsaView::NativeMsaView(QWidget *parent) : AbstractMsaView(parent)
{
    abstractTextRenderer_ = new TextPixmapRenderer(font(), zoom(), this);
    charWidth_ = abstractTextRenderer_->width();
    charHeight_ = abstractTextRenderer_->height();

    connect(this, SIGNAL(fontChanged()), this, SLOT(onAbstractMsaViewFontChanged()));
    connect(this, SIGNAL(zoomChanged(double)), this, SLOT(onAbstractMsaViewZoomChanged(double)));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented protected methods
/**
  * Draw the background using painter; empty stub because QWidget auto fills the background.
  *
  * @param painter [QPainter *]
  */
void NativeMsaView::drawBackground(QPainter *painter) const
{
    ASSERT(painter);
}

/**
  *
  *
  * @param origin [const QPointF &]
  * @param msaRect [const QRect &]
  * @param painter [QPainter *]
  */
void NativeMsaView::drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter)
{
    ASSERT(painter);
    renderMsaRegion(origin, msaRect, abstractTextRenderer_, *painter);
}

/**
  *
  *
  * @param rect [const QRectF &]
  * @param painter [QPainter *]
  */
void NativeMsaView::drawSelection(const QRectF &rect, QPainter *painter) const
{
    ASSERT(painter);

    QPen pen(Qt::black, qMax(1, qMin(2, qRound(zoom()))));

    // Draw the current mouse point
    painter->setPen(pen);
    painter->setBrush(QColor(0, 0, 0, 96));
    painter->drawRect(rect);
}

/**
  *
  *
  * @param rect [const QRectF &]
  * @param painter [QPainter *]
  */
void NativeMsaView::drawEditCursor(const QRectF &rect, QPainter *painter) const
{
    ASSERT(painter);
}

/**
  *
  *
  * @param rect [const QRectF &]
  * @param painter [QPainter *]
  */
void NativeMsaView::drawMouseActivePoint(const QRectF &rect, QPainter *painter) const
{
    ASSERT(painter);

    QPen pen(Qt::black, qMax(1, qMin(2, qRound(zoom()))));

    // Draw the current mouse point
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
}

void NativeMsaView::drawGapInsertionLine(qreal x, QPainter *painter) const
{
    ASSERT(painter);

    QPen pen(Qt::black, 2);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QPointF(x, 0), QPointF(x, height()));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void NativeMsaView::onAbstractMsaViewFontChanged()
{
    // By clearing the oldMsaRegionClip_, we ensure that the entire region is re-rendered
    oldMsaRegionClip_ = Rect();
    abstractTextRenderer_->setFont(font());
    charWidth_ = abstractTextRenderer_->width();
    charHeight_ = abstractTextRenderer_->height();
}

void NativeMsaView::onAbstractMsaViewZoomChanged(double zoom)
{
    // By clearing the oldMsaRegionClip_, we ensure that the entire region is re-rendered
    oldMsaRegionClip_ = Rect();
    abstractTextRenderer_->setScale(zoom);
    charWidth_ = abstractTextRenderer_->width();
    charHeight_ = abstractTextRenderer_->height();
}








// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/**
  * @param newMsaRegionClip [const Rect &]
  */
void SinglePixmapNativeMsaView::updateMsaRegionPixmap(const MsaRect &newMsaRegionClip)
{
    if (oldMsaRegionClip_ == newMsaRegionClip)
        return;

    if (oldMsaRegionClip_.contains(newMsaRegionClip))
    {
        // Simply need to clip out parts that are no longer visible
        // Ugh - only can copy at the pixel level - may cause some unwanted artifacts for floating point renderings
        //
        // Note: For the width and height, originally, just used the (newMsaRegion.{width,height} + 1) * char{Width_,Height_}
        //       however, when dealing with floating point precision, this misses any partial pixels because copy is
        //       restricted to integer sizes floors when casting to an integer. By ceil'ing it, we ensure that any partial
        //       pixels are copied as well.
        msaRegionClipPixmap_ = msaRegionClipPixmap_.copy((newMsaRegionClip.left() - oldMsaRegionClip_.left()) * charWidth_,
                                                         (newMsaRegionClip.top() - oldMsaRegionClip_.top()) * charHeight_,
                                                         newMsaRegionClip.width() * charWidth_,
                                                         newMsaRegionClip.height() * charHeight_);
        oldMsaRegionClip_ = newMsaRegionClip;
        return;
    }

    // Helper pixmap
    QPixmap regionPixmap = msaRegionClipPixmap_;

    // Resize the canvas to appropriately fit the new size if the size is different
    if (oldMsaRegionClip_.size() != newMsaRegionClip.size())
    {
        QRectF rect = pointRectMapper()->msaRectToCanvasRect(newMsaRegionClip);
        regionPixmap = QPixmap(QSize(static_cast<int>(ceil(rect.width())),
                                     static_cast<int>(ceil(rect.height()))));
    }

    QPainter painter(&regionPixmap);
    MsaRect intersection = oldMsaRegionClip_.intersection(newMsaRegionClip);
    if (intersection.isValid())
    {
        int nTopRows = qMax(0, intersection.top() - newMsaRegionClip.top());
        int nLeftRows = intersection.left() - newMsaRegionClip.left();
        int nBottomRows = newMsaRegionClip.bottom() - intersection.bottom();
        int nRightRows = newMsaRegionClip.right() - intersection.right();

        int dx = newMsaRegionClip.left() - oldMsaRegionClip_.left();
        int dy = newMsaRegionClip.top() - oldMsaRegionClip_.top();

//        qDebug() << "Top:" << nTopRows << "Left:" << nLeftRows << "Right:" << nRightRows << "Bottom:" << nBottomRows << "Dx:" << dx << "Dy:" << dy;

        // Draw the intersection - must do this first because the msaRegionClipPixmap_ (still points to the old
        // rendering) likely contains other obsolete data
        painter.drawPixmap(QPointF(-dx * charWidth_, -dy * charHeight_), msaRegionClipPixmap_);

        // Top rows
        if (nTopRows > 0)
            renderMsaRegion(QPointF(0, 0),
                            MsaRect(newMsaRegionClip.left(), newMsaRegionClip.top(), newMsaRegionClip.width(), nTopRows),
                            abstractTextRenderer(),
                            painter);

        // Left rows
        if (nLeftRows > 0)
            renderMsaRegion(QPointF(0, nTopRows * charHeight_),
                            MsaRect(newMsaRegionClip.left(), intersection.top(), nLeftRows, intersection.height()),
                            abstractTextRenderer(),
                            painter);

        // Right rows - checked
        if (nRightRows > 0)
        {
            ASSERT(intersection.right() >= newMsaRegionClip.left());
            renderMsaRegion(QPointF((intersection.right() - newMsaRegionClip.left() + 1) * charWidth_, nTopRows * charHeight_),
                            MsaRect(intersection.right() + 1, intersection.top(), nRightRows, intersection.height()),
                            abstractTextRenderer(),
                            painter);
        }

        // Bottom rows - checked
        if (nBottomRows > 0)
        {
            ASSERT(intersection.bottom() >= newMsaRegionClip.top());
            renderMsaRegion(QPointF(0, (intersection.bottom() - newMsaRegionClip.top() + 1) * charHeight_),
                            MsaRect(newMsaRegionClip.left(), intersection.bottom() + 1, newMsaRegionClip.width(), nBottomRows),
                            abstractTextRenderer(),
                            painter);
        }
    }
    else
    {
        // Completely new rendering!
        renderMsaRegion(QPointF(0, 0), newMsaRegionClip, abstractTextRenderer(), painter);
    }
    painter.end();

    // Point msaRegionPixmap to the right pixmap
    msaRegionClipPixmap_ = regionPixmap;

    // Update the oldMsaRegionClip
    oldMsaRegionClip_ = newMsaRegionClip;
}

void SinglePixmapNativeMsaView::repaintColumns(int start, int end)
{
    if (start > oldMsaRegionClip_.right() || end < oldMsaRegionClip_.left())
        return;

    int startColumn = qMax(start, oldMsaRegionClip_.left());
    int stopColumn = qMin(end, oldMsaRegionClip_.right());

    QPainter painter(&msaRegionClipPixmap_);
    renderMsaRegion(QPointF((startColumn - oldMsaRegionClip_.left()) * charWidth_, 0),
                    MsaRect(startColumn, oldMsaRegionClip_.top(), stopColumn - startColumn + 1, oldMsaRegionClip_.bottom()),
                    abstractTextRenderer(),
                    painter);
}

void SinglePixmapNativeMsaView::onMsaRegionSlid(int left, int top, int right, int bottom, int delta, int finalLeft, int finalRight)
{
    onMsaExtendOrTrimFinished(qMin(left, finalLeft), qMax(right, finalRight));
    return;

    // The following code is for cases where only the columns that have changed positions need to be redrawn.
    // In other words, when the sliding operation only affects the selected area and its previous location - not any
    // columns outside its selection. For example, when drawing VISSA colors. A contrasting example is Clustal.
    ASSERT(delta != 0);

    MsaRect dirtyRect = MsaRect(QPoint(qMin(left, finalLeft), top),
                                QPoint(qMax(right, finalRight), bottom)).normalized();

    MsaRect dirtyIntersection = dirtyRect.intersection(oldMsaRegionClip_);
    // No need to do anything if this event occurred outside the pixmap bounds
    if (!dirtyIntersection.isValid())
        return;

    QPainter painter(&msaRegionClipPixmap_);
    // Must re-render entire region because the positional color provider data may have changed :|
    renderMsaRegion(QPointF((dirtyIntersection.left() - oldMsaRegionClip_.left()) * charWidth_,
                            (dirtyIntersection.top() - oldMsaRegionClip_.top()) * charHeight_),
                    dirtyIntersection,
                    abstractTextRenderer(),
                    painter);
    painter.end();
}

void SinglePixmapNativeMsaView::onMsaGapColumnsInserted(int column, int count)
{
    // Call the parent for processing
    NativeMsaView::onMsaGapColumnsInserted(column, count);

//    qDebug() << "Insert gap" << column << count << oldMsaRegionClip_;
//    qDebug() << "Draw location" << column + count - oldMsaRegionClip_.left();
//    qDebug() << "Source" << column - oldMsaRegionClip_.left() << " .. " << oldMsaRegionClip_.right() - column - count;
//    qDebug() << msa_->length();

    if (column < oldMsaRegionClip_.left() - 1 || column > oldMsaRegionClip_.right())
        return;

    // Copy old area and then render the new gap columns
    QPainter painter(&msaRegionClipPixmap_);
    if (column < msa_->length() - count)
    {
        painter.drawPixmap(QPointF((column + count - oldMsaRegionClip_.left()) * charWidth_, 0.),
                           msaRegionClipPixmap_.copy((column - oldMsaRegionClip_.left()) * charWidth_,
                                                     0.,
                                                     (oldMsaRegionClip_.right() - column - count + 1) * charWidth_,
                                                     oldMsaRegionClip_.height() * charHeight_));
    }

    // Now render the new gap columns
    renderMsaRegion(QPointF((column - oldMsaRegionClip_.left()) * charWidth_, 0.),
                    MsaRect(column, oldMsaRegionClip_.top(), count, oldMsaRegionClip_.height()),
                    abstractTextRenderer(),
                    painter);
}

void SinglePixmapNativeMsaView::onMsaGapColumnsRemoved(int count)
{
    NativeMsaView::onMsaGapColumnsRemoved(count);

    // Re-render everything
    oldMsaRegionClip_ = MsaRect();
}

void SinglePixmapNativeMsaView::onMsaExtendOrTrimFinished(int start, int end)
{
    NativeMsaView::onMsaExtendOrTrimFinished(start, end);

    repaintColumns(start, end);
}

void SinglePixmapNativeMsaView::onMsaCollapsedLeft(const MsaRect &msaRect, int rightMostModifiedColumn)
{
    NativeMsaView::onMsaCollapsedLeft(msaRect, rightMostModifiedColumn);

    repaintColumns(msaRect.left(), rightMostModifiedColumn);
}

void SinglePixmapNativeMsaView::onMsaCollapsedRight(const MsaRect &msaRect, int leftMostModifiedColumn)
{
    NativeMsaView::onMsaCollapsedRight(msaRect, leftMostModifiedColumn);

    repaintColumns(leftMostModifiedColumn, msaRect.right());
}


/**
  * @param newMsaRegionClip [const Rect &]
  */
void PixmapPerSeqNativeMsaView::setMsaRegionClip(const Rect &newMsaRegionClip)
{
    Rect oldMsaRegionClip = msaRegionClip();
    AbstractMsaView::setMsaRegionClip(newMsaRegionClip);
    if (oldMsaRegionClip == newMsaRegionClip)
        return;

    qDebug() << oldMsaRegionClip;
    qDebug() << newMsaRegionClip;

    if (oldMsaRegionClip.isNull()
            || newMsaRegionClip.top() - oldMsaRegionClip.top() >= pixmaps_.count()
            || oldMsaRegionClip.bottom() - newMsaRegionClip.bottom() >= pixmaps_.count())
    {
        pixmaps_.clear();
        qDebug() << "Cranking new ones";
        for (int i=newMsaRegionClip.top(); i<=newMsaRegionClip.bottom(); ++i)
        {
            Rect renderRegion = Rect(1, i, newMsaRegionClip.right()-1, 0);

            QPixmap pixmap(static_cast<int>(ceil(newMsaRegionClip.right() * charWidth_)),
                           static_cast<int>(ceil(charHeight_)));
            QPainter painter(&pixmap);
            renderMsaRegion(QPointF(0, 0), renderRegion, abstractTextRenderer(), painter);
            painter.end();

            pixmaps_ << pixmap;
        }
        return;
    }

    if (newMsaRegionClip.top() < oldMsaRegionClip.top())
    {
        qDebug() << "new.top < old.top";
        for (int i=oldMsaRegionClip.top()-1; i>= newMsaRegionClip.top(); --i)
        {
            Rect renderRegion = Rect(1, i, newMsaRegionClip.right()-1, 0);

            QPixmap pixmap(static_cast<int>(ceil(newMsaRegionClip.right() * charWidth_)),
                           static_cast<int>(ceil(charHeight_)));
            QPainter painter(&pixmap);
            renderMsaRegion(QPointF(0, 0), renderRegion, abstractTextRenderer(), painter);
            painter.end();

            pixmaps_.push_front(pixmap);
        }
    }
    else if (newMsaRegionClip.top() > oldMsaRegionClip.top())
    {
        qDebug() << "new.top > old.top";
        for (int i=0, z= newMsaRegionClip.top() - oldMsaRegionClip.top(); i<z; ++i)
            pixmaps_.removeFirst();
    }

    // Now for the intermediate rows
    QSize size(static_cast<int>(ceil(newMsaRegionClip.right() * charWidth_)),
               static_cast<int>(ceil(charHeight_)));
    if (newMsaRegionClip.right() > oldMsaRegionClip.right())
    {
        QPointF renderOrigin(oldMsaRegionClip.right() * charWidth_, 0);
        qDebug() << "new.right > old.right" << renderOrigin << size;

        Rect inter = newMsaRegionClip.intersection(oldMsaRegionClip);
        if (inter.isNull())
            inter = newMsaRegionClip;

        qDebug() << inter;

        for (int i=inter.top(); i<= inter.bottom(); ++i)
        {
            // Resize the pixmap
            QPixmap pixmap = QPixmap(size);

            QPainter painter(&pixmap);
            painter.drawPixmap(0, 0, pixmaps_[i - newMsaRegionClip.top()]);

            renderMsaRegion(renderOrigin, Rect(oldMsaRegionClip.right()+1, i, newMsaRegionClip.right() - oldMsaRegionClip.right() - 1, 0), abstractTextRenderer(), painter);

            painter.end();

            pixmaps_[i - newMsaRegionClip.top()] = pixmap;
        }
    }
    else if (newMsaRegionClip.right() < oldMsaRegionClip.right())
    {
        qDebug() << "new.right < old.right";

        Rect inter = newMsaRegionClip.intersection(oldMsaRegionClip);
        if (inter.isNull())
            inter = newMsaRegionClip;

        for (int i=inter.top(); i<= inter.bottom(); ++i)
        {
            // Resize the pixmap
            QPixmap pixmap = QPixmap(size);

            QPainter painter(&pixmap);
            painter.drawPixmap(0, 0, pixmaps_[i - newMsaRegionClip.top()]);
            painter.end();

            pixmaps_[i - newMsaRegionClip.top()] = pixmap;
        }
    }

    if (newMsaRegionClip.bottom() > oldMsaRegionClip.bottom())
    {
        qDebug() << "new.bottom > old.bottom";
        for (int i=oldMsaRegionClip.bottom()+1; i<= newMsaRegionClip.bottom(); ++i)
        {
            Rect renderRegion = Rect(1, i, newMsaRegionClip.right()-1, 0);

            QPixmap pixmap(static_cast<int>(ceil(newMsaRegionClip.right() * charWidth_)),
                           static_cast<int>(ceil(charHeight_)));
            QPainter painter(&pixmap);
            renderMsaRegion(QPointF(0, 0), renderRegion, abstractTextRenderer(), painter);
            painter.end();

            pixmaps_ << pixmap;
        }
    }
    else if (newMsaRegionClip.bottom() < oldMsaRegionClip.bottom())
    {
        qDebug() << "new.bottom < old.bottom";
        for (int i=0, z= oldMsaRegionClip.bottom() - newMsaRegionClip.bottom(); i<z; ++i)
            pixmaps_.removeLast();
    }
}






// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public slots









void BlockNativeMsaView::drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter)
{
    int nVertChars = static_cast<double>(blockH_) / charHeight_;
    int nHorzChars = static_cast<double>(blockW_) / charWidth_;

    int top = (msaRect.top()-1) / nVertChars;
    int left = (msaRect.left()-1) / nHorzChars;

    int bottom = (msaRect.bottom()-1) / nVertChars;
    int right = (msaRect.right()-1) / nHorzChars;

    qDebug() << left << top << right << bottom << nHorzChars << nVertChars;
    qDebug() << pixmaps_.count();

    QPointF renderPoint(-(((msaRect.left()-1) % nHorzChars) * charWidth_) + origin.x(),
                        -(((msaRect.top()-1) % nVertChars) * charHeight_) + origin.y());
    qDebug() << "Msa rect:" << msaRect;
    qDebug() << "Render point:" << renderPoint;
    for (int i=top, v=0; i<= bottom; ++i, ++v)
    {
        qDebug() << "Number pixmaps for" << v << pixmaps_[v].count();
        for (int j=left, u=0; j<= right; ++j, ++u)
        {
            QPointF renderPoint2(renderPoint.x() + u * nHorzChars * charWidth_,
                                 renderPoint.y() + v * nVertChars * charHeight_);
            qDebug() << "Renderpoint 2" << renderPoint2;
            painter->drawPixmap(renderPoint2, pixmaps_[v][u]);
        }
    }
}

void BlockNativeMsaView::setMsaRegionClip(const Rect &newMsaRegionClip)
{
    Rect oldMsaRegionClip = msaRegionClip();
    AbstractMsaView::setMsaRegionClip(newMsaRegionClip);
    if (oldMsaRegionClip == newMsaRegionClip)
        return;

    qDebug() << oldMsaRegionClip;
    qDebug() << newMsaRegionClip;

    int nVertChars = static_cast<double>(blockH_) / charHeight_;
    int nHorzChars = static_cast<double>(blockW_) / charWidth_;

    int newTop = (newMsaRegionClip.top()-1) / nVertChars;
    int newLeft = (newMsaRegionClip.left()-1) / nHorzChars;

    int newBottom = (newMsaRegionClip.bottom()-1) / nVertChars;
    int newRight = (newMsaRegionClip.right()-1) / nHorzChars;

    qDebug() << "Olds:" << left_ << top_ << right_ << bottom_;
    qDebug() << "news:" << newLeft << newTop << newRight << newBottom;

    if (oldMsaRegionClip.isNull()
        || newTop > bottom_
        || newBottom < top_
        || newLeft > right_
        || newRight < left_)
    {
        for (int i=0; i< pixmaps_.count(); ++i)
            for (int j=0; j< pixmaps_[i].count(); ++j)
                prepixes_ << pixmaps_[i][j];
        pixmaps_.clear();
        qDebug() << "Cranking new ones";

        top_ = (newMsaRegionClip.top()-1) / nVertChars;
        left_ = (newMsaRegionClip.left()-1) / nHorzChars;

        bottom_ = (newMsaRegionClip.bottom()-1) / nVertChars;
        right_ = (newMsaRegionClip.right()-1) / nHorzChars;

        for (int i=top_; i<= bottom_; ++i)
        {
            pixmaps_ << QList<QPixmap>();
            for (int j=left_; j<= right_; ++j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                QPixmap pixmap = prepixes_.pop();
                pixmap.fill();
                QPainter painter(&pixmap);
                renderMsaRegion(QPointF(0, 0), renderRegion, abstractTextRenderer(), painter);
                painter.end();

                pixmaps_.last() << pixmap;
            }
        }

        return;
    }

    if (newTop < top_)
    {
        qDebug() << "new.top < old.top";
        for (int i=top_-1; i>= newTop; --i)
        {
            pixmaps_.push_front(QList<QPixmap>());
            for (int j=newLeft; j<= newRight; ++j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                QPixmap pixmap = prepixes_.pop();
                pixmap.fill();
                QPainter painter(&pixmap);
                renderMsaRegion(QPointF(0, 0), renderRegion, abstractTextRenderer(), painter);
                painter.end();

                pixmaps_.first() << pixmap;
            }
        }
    }
    else if (newTop > top_)
    {
        qDebug() << "new.top > old.top";
        for (int i=0, z= newTop - top_; i<z; ++i)
        {
            QList<QPixmap> plist = pixmaps_.takeFirst();
            for (int i=0; i< plist.count(); ++i)
                prepixes_ << plist.at(i);
        }
    }

    if (newBottom > bottom_)
    {
        qDebug() << "new.bottom > old.bottom";

        for (int i=bottom_+1; i<= newBottom; ++i)
        {
            pixmaps_ << QList<QPixmap>();
            for (int j=newLeft; j<= newRight; ++j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                QPixmap pixmap = prepixes_.pop();
                pixmap.fill();
                QPainter painter(&pixmap);
                renderMsaRegion(QPointF(0, 0), renderRegion, abstractTextRenderer(), painter);
                painter.end();

                pixmaps_.last() << pixmap;
            }
        }
    }
    else if (newBottom < bottom_)
    {
        qDebug() << "new.bottom < old.bottom";
        for (int i=0, z= bottom_ - newBottom; i<z; ++i)
        {
            QList<QPixmap> plist = pixmaps_.takeLast();
            for (int i=0; i< plist.count(); ++i)
                prepixes_ << plist.at(i);
        }
    }

    if (newLeft < left_)
    {
        qDebug() << "newLeft < left_";
        int list_index = (newTop < top_) ? top_ - newTop : 0;

        for (int i=qMax(newTop, top_); i<= qMin(newBottom, bottom_); ++i)
        {
            for (int j=left_-1; j>= newLeft; --j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                QPixmap pixmap = prepixes_.pop();
                pixmap.fill();
                QPainter painter(&pixmap);
                renderMsaRegion(QPointF(0, 0), renderRegion, abstractTextRenderer(), painter);
                painter.end();

                pixmaps_[list_index].push_front(pixmap);
            }
            ++list_index;
        }
    }
    else if (newLeft > left_)
    {
        qDebug() << "newLeft > left_";
        int list_index = (newTop < top_) ? top_ - newTop : 0;

        for (int i=qMax(newTop, top_); i<= qMin(newBottom, bottom_); ++i)
        {
            for (int j=left_; j< newLeft; ++j)
                prepixes_ << pixmaps_[list_index].takeFirst();
            ++list_index;
        }
    }

    if (newRight > right_)
    {
        qDebug() << "newRight > right_";
        int list_index = (newTop < top_) ? top_ - newTop : 0;

        for (int i=qMax(newTop, top_); i<= qMin(newBottom, bottom_); ++i)
        {
            for (int j=right_+1; j<= newRight; ++j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                QPixmap pixmap = prepixes_.pop();
                pixmap.fill();
                QPainter painter(&pixmap);
                renderMsaRegion(QPointF(0, 0), renderRegion, abstractTextRenderer(), painter);
                painter.end();

                pixmaps_[list_index] << pixmap;
            }
            ++list_index;
        }
    }
    else if (newRight < right_)
    {
        qDebug() << "newRight < right_";
        int list_index = (newTop < top_) ? top_ - newTop : 0;

        for (int i=qMax(newTop, top_); i<= qMin(newBottom, bottom_); ++i)
        {
            for (int j=newRight; j< right_; ++j)
                prepixes_ << pixmaps_[list_index].takeLast();
            ++list_index;
        }
    }

    top_ = newTop;
    bottom_ = newBottom;
    left_ = newLeft;
    right_ = newRight;
}







void OnePixmapBlockNativeMsaView::drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter)
{
    int nVertChars = static_cast<double>(blockH_) / charHeight_;
    int nHorzChars = static_cast<double>(blockW_) / charWidth_;

    int top = (msaRect.top()-1) / nVertChars;
    int left = (msaRect.left()-1) / nHorzChars;

    int bottom = (msaRect.bottom()-1) / nVertChars;
    int right = (msaRect.right()-1) / nHorzChars;

    qDebug() << left << top << right << bottom << nHorzChars << nVertChars;

    QPointF renderPoint(-(((msaRect.left()-1) % nHorzChars) * charWidth_) + origin.x(),
                        -(((msaRect.top()-1) % nVertChars) * charHeight_) + origin.y());
    qDebug() << "Msa rect:" << msaRect;
    qDebug() << "Render point:" << renderPoint;
    for (int i=top, v=0; i<= bottom; ++i, ++v)
    {
        for (int j=left, u=0; j<= right; ++j, ++u)
        {
            QPointF renderPoint2(renderPoint.x() + u * nHorzChars * charWidth_,
                                 renderPoint.y() + v * nVertChars * charHeight_);
            qDebug() << "Renderpoint 2" << renderPoint2;

            Rect sourceRect = lookup_[i].value(j);
            QRect source(sourceRect.left(), sourceRect.top(), sourceRect.width(), sourceRect.height());
            painter->drawPixmap(renderPoint2, masterPixmap_, source);
        }
    }
}

void OnePixmapBlockNativeMsaView::setMsaRegionClip(const Rect &newMsaRegionClip)
{
    Rect oldMsaRegionClip = msaRegionClip();
    AbstractMsaView::setMsaRegionClip(newMsaRegionClip);
    if (oldMsaRegionClip == newMsaRegionClip)
        return;

    qDebug() << oldMsaRegionClip;
    qDebug() << newMsaRegionClip;

    int nVertChars = static_cast<double>(blockH_) / charHeight_;
    int nHorzChars = static_cast<double>(blockW_) / charWidth_;

    int newTop = (newMsaRegionClip.top()-1) / nVertChars;
    int newLeft = (newMsaRegionClip.left()-1) / nHorzChars;

    int newBottom = (newMsaRegionClip.bottom()-1) / nVertChars;
    int newRight = (newMsaRegionClip.right()-1) / nHorzChars;

    qDebug() << "Olds:" << left_ << top_ << right_ << bottom_;
    qDebug() << "news:" << newLeft << newTop << newRight << newBottom;

    if (oldMsaRegionClip.isNull()
        || newTop > bottom_
        || newBottom < top_
        || newLeft > right_
        || newRight < left_)
    {
        topAvailRect_ = rects_.count() - 1;
        lookup_.clear();
        qDebug() << "Cranking new ones";

        top_ = (newMsaRegionClip.top()-1) / nVertChars;
        left_ = (newMsaRegionClip.left()-1) / nHorzChars;

        bottom_ = (newMsaRegionClip.bottom()-1) / nVertChars;
        right_ = (newMsaRegionClip.right()-1) / nHorzChars;

        QPainter painter(&masterPixmap_);
        for (int i=top_; i<= bottom_; ++i)
        {
            if (!lookup_.contains(i))
                lookup_.insert(i, QHash<int, Rect>());

            for (int j=left_; j<= right_; ++j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                const Rect destRect = rects_.at(topAvailRect_);
                qDebug() << destRect;
                lookup_[i].insert(j, destRect);
                --topAvailRect_;
                painter.fillRect(destRect.left(), destRect.top(), destRect.width(), destRect.height(), Qt::white);
                renderMsaRegion(destRect.topLeft(), renderRegion, abstractTextRenderer(), painter);
            }
        }
        painter.end();

        masterPixmap_.save("master_pixmap.png");

        return;
    }

    QPainter painter(&masterPixmap_);
    if (newTop < top_)
    {
        qDebug() << "new.top < old.top";
        for (int i=top_-1; i>= newTop; --i)
        {
            if (!lookup_.contains(i))
                lookup_.insert(i, QHash<int, Rect>());
            for (int j=newLeft; j<= newRight; ++j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                const Rect destRect = rects_.at(topAvailRect_);
                lookup_[i].insert(j, destRect);
                --topAvailRect_;
                painter.fillRect(destRect.left(), destRect.top(), destRect.width(), destRect.height(), Qt::white);
                renderMsaRegion(destRect.topLeft(), renderRegion, abstractTextRenderer(), painter);
            }
        }
    }
    else if (newTop > top_)
    {
        qDebug() << "new.top > old.top";
        for (int i=top_; i<newTop; ++i)
        {
            foreach (const Rect &rect, lookup_[i].values())
            {
                rects_[topAvailRect_+1] = rect;
                ++topAvailRect_;
            }

            lookup_.remove(i);
        }
    }

    if (newBottom > bottom_)
    {
        qDebug() << "new.bottom > old.bottom";

        for (int i=bottom_+1; i<= newBottom; ++i)
        {
            if (!lookup_.contains(i))
                lookup_.insert(i, QHash<int, Rect>());
            for (int j=newLeft; j<= newRight; ++j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                const Rect destRect = rects_.at(topAvailRect_);
                lookup_[i].insert(j, destRect);
                --topAvailRect_;
                painter.fillRect(destRect.left(), destRect.top(), destRect.width(), destRect.height(), Qt::white);
                renderMsaRegion(destRect.topLeft(), renderRegion, abstractTextRenderer(), painter);
            }
        }
    }
    else if (newBottom < bottom_)
    {
        qDebug() << "new.bottom < old.bottom";
        for (int i=newBottom+1; i<=bottom_; ++i)
        {
            foreach (const Rect &rect, lookup_[i].values())
            {
                rects_[topAvailRect_+1] = rect;
                ++topAvailRect_;
            }

            lookup_.remove(i);
        }
    }

    if (newLeft < left_)
    {
        qDebug() << "newLeft < left_";
        for (int i=qMax(newTop, top_); i<= qMin(newBottom, bottom_); ++i)
        {
            if (!lookup_.contains(i))
                lookup_.insert(i, QHash<int, Rect>());
            for (int j=left_-1; j>= newLeft; --j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                const Rect destRect = rects_.at(topAvailRect_);
                lookup_[i].insert(j, destRect);
                --topAvailRect_;
                painter.fillRect(destRect.left(), destRect.top(), destRect.width(), destRect.height(), Qt::white);
                renderMsaRegion(destRect.topLeft(), renderRegion, abstractTextRenderer(), painter);
            }
        }
    }
    else if (newLeft > left_)
    {
        qDebug() << "newLeft > left_";
        for (int i=qMax(newTop, top_); i<= qMin(newBottom, bottom_); ++i)
        {
            for (int j=left_; j< newLeft; ++j)
            {
                rects_[topAvailRect_+1] = lookup_[i].take(j);
                ++topAvailRect_;
            }
        }
    }

    if (newRight > right_)
    {
        qDebug() << "newRight > right_";
        for (int i=qMax(newTop, top_); i<= qMin(newBottom, bottom_); ++i)
        {
            for (int j=right_+1; j<= newRight; ++j)
            {
                // Render the block
                Rect renderRegion = Rect(j * nHorzChars + 1, i * nVertChars + 1, nHorzChars - 1, nVertChars - 1);
                if (renderRegion.right() > msa_->length())
                    renderRegion.setRight(msa_->length());
                if (renderRegion.bottom() > msa_->subseqCount())
                    renderRegion.setBottom(msa_->subseqCount());

                const Rect destRect = rects_.at(topAvailRect_);
                lookup_[i].insert(j, destRect);
                --topAvailRect_;
                painter.fillRect(destRect.left(), destRect.top(), destRect.width(), destRect.height(), Qt::white);
                renderMsaRegion(destRect.topLeft(), renderRegion, abstractTextRenderer(), painter);
            }
        }
    }
    else if (newRight < right_)
    {
        qDebug() << "newRight < right_";
        for (int i=qMax(newTop, top_); i<= qMin(newBottom, bottom_); ++i)
        {
            for (int j=newRight+1; j<= right_; ++j)
            {
                rects_[topAvailRect_+1] = lookup_[i].take(j);
                ++topAvailRect_;
            }
        }
    }

    top_ = newTop;
    bottom_ = newBottom;
    left_ = newLeft;
    right_ = newRight;
}





