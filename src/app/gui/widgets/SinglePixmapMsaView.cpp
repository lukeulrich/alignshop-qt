/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <cmath>

#include <QtGui/QPainter>

#include "SinglePixmapMsaView.h"
#include "../../core/ObservableMsa.h"
#include "../../core/util/ClosedIntRange.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
SinglePixmapMsaView::SinglePixmapMsaView(QWidget *parent)
    : NativeMsaView(parent)
{
    connect(this, SIGNAL(colorProviderChanged()), SLOT(clearCache()));
    connect(this, SIGNAL(fontChanged()), SLOT(clearCache()));
    connect(this, SIGNAL(zoomChanged(double)), SLOT(clearCache()));
    connect(this, SIGNAL(msaChanged()), SLOT(clearCache()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void SinglePixmapMsaView::clearCache()
{
    cachedPixmap_ = QPixmap();
    cachedMsaRect_ = PosiRect();
}

/**
  * @param msaRow [int]
  */
void SinglePixmapMsaView::repaintRow(int msaRow)
{
    ASSERT(msaRow > 0 && msaRow <= msa()->rowCount());
    if (msaRow < cachedMsaRect_.top() || msaRow > cachedMsaRect_.bottom())
        return;

    QPainter painter(&cachedPixmap_);
    AbstractMsaView::renderMsaRegion(QPointF(0, (msaRow - cachedMsaRect_.top()) * charHeight()),
                    PosiRect(cachedMsaRect_.left(), msaRow, cachedMsaRect_.width(), 1),
                    renderEngine(),
                    &painter);
    viewport()->update();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param origin [const QPointF &]
  * @param msaRect [const PosiRect &]
  * @param painter [QPainter *]
  */
void SinglePixmapMsaView::drawMsa(const QPointF &origin, const PosiRect &msaRect, QPainter *painter)
{
    if (msaRect != cachedMsaRect_)
        updateMsaCachedPixmap(msaRect);

    painter->drawPixmap(origin, cachedPixmap_);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected slots
/**
  * @param columns [const ClosedIntRange &]
  */
void SinglePixmapMsaView::onMsaGapColumnsInserted(const ClosedIntRange &columns)
{
    NativeMsaView::onMsaGapColumnsInserted(columns);

//    qDebug() << "Insert gap" << columns.begin_ << columns.end_ << cachedMsaRect_;
//    qDebug() << "Draw location" << columns.begin_ + columns.length() - cachedMsaRect_.left();
//    qDebug() << "Source" << columns.begin_ - cachedMsaRect_.left() << " .. " << cachedMsaRect_.right() - columns.begin_ - columns.length();
//    qDebug() << msa()->length();

    if (columns.begin_ > cachedMsaRect_.right())
        return;

    if (columns.begin_ < cachedMsaRect_.left())
    {
        // Update the cached msa rect and then return
        cachedMsaRect_.moveLeft(cachedMsaRect_.left() + columns.length());
        return;
    }

    // Copy old area and then render the new gap columns
    QPainter painter(&cachedPixmap_);
    if (columns.begin_ < msa()->length() - columns.length())
    {
        painter.drawPixmap(QPointF((columns.begin_ + columns.length() - cachedMsaRect_.left()) * charWidth(), 0.),
                           cachedPixmap_.copy((columns.begin_ - cachedMsaRect_.left()) * charWidth(),
                                              0.,
                                              (cachedMsaRect_.right() - columns.begin_ - columns.length() + 1) * charWidth(),
                                              cachedMsaRect_.height() * charHeight()));
    }

    // Now render the new gap columns
    renderMsaRegion(QPointF((columns.begin_ - cachedMsaRect_.left()) * charWidth(), 0.),
                    PosiRect(columns.begin_, cachedMsaRect_.top(), columns.length(), cachedMsaRect_.height()),
                    renderEngine(),
                    &painter);
}

/**
  * @param columnRanges [const QVector<ClosedIntRange> &]
  */
void SinglePixmapMsaView::onMsaGapColumnsRemoved(const QVector<ClosedIntRange> &columnRanges)
{
    NativeMsaView::onMsaGapColumnsRemoved(columnRanges);

    // Re-render everything
    cachedMsaRect_ = PosiRect();
}

/**
  * @param msaRect [const PosiRect &]
  * @param delta [int]
  * @param finalRange [const ClosedIntRange &]
  */
void SinglePixmapMsaView::onMsaRectangleSlid(const PosiRect &msaRect, int delta, const ClosedIntRange &finalRange)
{
    NativeMsaView::onMsaRectangleSlid(msaRect, delta, finalRange);

    PosiRect normalizedRect = msaRect.normalized();
    repaintColumns(ClosedIntRange(qMin(normalizedRect.left(), finalRange.begin_), qMax(normalizedRect.right(), finalRange.end_)));
}

/**
  * @param msaRect [const PosiRect &]
  */
void SinglePixmapMsaView::onMsaCollapsedLeft(const PosiRect &msaRect)
{
    NativeMsaView::onMsaCollapsedLeft(msaRect);

    repaintColumns(msaRect.normalized().horizontalRange());
}

/**
  * @param msaRect [const PosiRect &]
  */
void SinglePixmapMsaView::onMsaCollapsedRight(const PosiRect &msaRect)
{
    NativeMsaView::onMsaCollapsedRight(msaRect);

    repaintColumns(msaRect.normalized().horizontalRange());
}

/**
  * @param rows [const ClosedIntRange &]
  */
void SinglePixmapMsaView::onMsaRowsInserted(const ClosedIntRange &rows)
{
    NativeMsaView::onMsaRowsInserted(rows);

    cachedPixmap_ = QPixmap();
    cachedMsaRect_ = PosiRect();
}

void SinglePixmapMsaView::onMsaRowsMoved(const ClosedIntRange &rows, int finalRow)
{
    NativeMsaView::onMsaRowsMoved(rows, finalRow);

    cachedPixmap_ = QPixmap();
    cachedMsaRect_ = PosiRect();
}

/**
  * @param rows [const ClosedIntRange &]
  */
void SinglePixmapMsaView::onMsaRowsRemoved(const ClosedIntRange &rows)
{
    NativeMsaView::onMsaRowsRemoved(rows);

    cachedPixmap_ = QPixmap();
    cachedMsaRect_ = PosiRect();
}

/**
  */
void SinglePixmapMsaView::onMsaRowsSorted()
{
    NativeMsaView::onMsaRowsSorted();

    cachedPixmap_ = QPixmap();
    cachedMsaRect_ = PosiRect();
}

/**
  * @param subseqChangePods [const SubseqChangePodVector &]
  */
void SinglePixmapMsaView::onMsaSubseqsChanged(const SubseqChangePodVector &subseqChangePods)
{
    NativeMsaView::onMsaSubseqsChanged(subseqChangePods);

    if (subseqChangePods.isEmpty())
        return;

    // Extract the minimum and maximum columns affected
    ClosedIntRange affectedColumns(99999999, 0);
    foreach (const SubseqChangePod &pod, subseqChangePods)
    {
        if (pod.columns_.begin_ < affectedColumns.begin_)
            affectedColumns.begin_ = pod.columns_.begin_;

        if (pod.columns_.end_ > affectedColumns.end_)
            affectedColumns.end_ = pod.columns_.end_;
    }

    repaintColumns(affectedColumns);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param columns [const ClosedIntRange &]
  */
void SinglePixmapMsaView::repaintColumns(const ClosedIntRange &columns)
{
    if (columns.begin_ > cachedMsaRect_.right() || columns.end_ < cachedMsaRect_.left())
        return;

    int startColumn = qMax(columns.begin_, cachedMsaRect_.left());
    int stopColumn = qMin(columns.end_, cachedMsaRect_.right());

    QPainter painter(&cachedPixmap_);
    AbstractMsaView::renderMsaRegion(QPointF((startColumn - cachedMsaRect_.left()) * charWidth(), 0),
                    PosiRect(startColumn, cachedMsaRect_.top(), stopColumn - startColumn + 1, cachedMsaRect_.height()),
                    renderEngine(),
                    &painter);
}

/**
  * Only intended to be called for panning operations or changes that do not affect the overall dimensions of the msa.
  * The other signals and slots should respond to those appropriately and afterwards update the cachedMsaRect_ so that
  * this method is never called when those changes take place.
  *
  * @param msaRect [const PosiRect &]
  */
void SinglePixmapMsaView::updateMsaCachedPixmap(const PosiRect &msaRect)
{
    if (cachedMsaRect_.contains(msaRect))
    {
        // Simply need to clip out parts that are no longer visible
        // Ugh - only can copy at the pixel level - may cause some unwanted artifacts for floating point renderings
        //
        // Note: For the width and height, originally, just used the (newMsaRegion.{width,height} + 1) * char{Width_,Height_}
        //       however, when dealing with floating point precision, this misses any partial pixels because copy is
        //       restricted to integer sizes floors when casting to an integer. By ceil'ing it, we ensure that any partial
        //       pixels are copied as well.
        cachedPixmap_ = cachedPixmap_.copy((msaRect.left() - cachedMsaRect_.left()) * charWidth(),
                                           (msaRect.top() - cachedMsaRect_.top()) * charHeight(),
                                           msaRect.width() * charWidth(),
                                           msaRect.height() * charHeight());
        cachedMsaRect_ = msaRect;
        return;
    }

    // Helper pixmap
    QPixmap regionPixmap = cachedPixmap_;

    // Resize the canvas to appropriately fit the new size if the size is different
    if (cachedMsaRect_.size() != msaRect.size())
    {
        QRectF rect = pointRectMapper()->msaRectToCanvasRect(msaRect);
        regionPixmap = QPixmap(QSize(static_cast<int>(ceil(rect.width())),
                                     static_cast<int>(ceil(rect.height()))));
    }

    QPainter painter(&regionPixmap);
    PosiRect intersection = cachedMsaRect_.intersection(msaRect);
    if (intersection.isValid())
    {
        int nTopRows = qMax(0, intersection.top() - msaRect.top());
        int nLeftRows = intersection.left() - msaRect.left();
        int nBottomRows = msaRect.bottom() - intersection.bottom();
        int nRightRows = msaRect.right() - intersection.right();

        int dx = msaRect.left() - cachedMsaRect_.left();
        int dy = msaRect.top() - cachedMsaRect_.top();

//        qDebug() << "Top:" << nTopRows << "Left:" << nLeftRows << "Right:" << nRightRows << "Bottom:" << nBottomRows << "Dx:" << dx << "Dy:" << dy;

        // Draw the intersection - must do this first because the cachedPixmap_ (still points to the old
        // rendering) likely contains other obsolete data
        painter.drawPixmap(QPointF(-dx * charWidth(), -dy * charHeight()), cachedPixmap_);

        // Top rows
        if (nTopRows > 0)
            renderMsaRegion(QPointF(0, 0),
                            PosiRect(msaRect.left(), msaRect.top(), msaRect.width(), nTopRows),
                            renderEngine(),
                            &painter);

        // Left rows
        if (nLeftRows > 0)
            renderMsaRegion(QPointF(0, nTopRows * charHeight()),
                            PosiRect(msaRect.left(), intersection.top(), nLeftRows, intersection.height()),
                            renderEngine(),
                            &painter);

        // Right rows - checked
        if (nRightRows > 0)
        {
            ASSERT(intersection.right() >= msaRect.left());
            renderMsaRegion(QPointF((intersection.right() - msaRect.left() + 1) * charWidth(), nTopRows * charHeight()),
                            PosiRect(intersection.right() + 1, intersection.top(), nRightRows, intersection.height()),
                            renderEngine(),
                            &painter);
        }

        // Bottom rows - checked
        if (nBottomRows > 0)
        {
            ASSERT(intersection.bottom() >= msaRect.top());
            renderMsaRegion(QPointF(0, (intersection.bottom() - msaRect.top() + 1) * charHeight()),
                            PosiRect(msaRect.left(), intersection.bottom() + 1, msaRect.width(), nBottomRows),
                            renderEngine(),
                            &painter);
        }
    }
    else
    {
        // Completely new rendering!
        renderMsaRegion(QPointF(0, 0), msaRect, renderEngine(), &painter);
    }
    painter.end();

    cachedPixmap_ = regionPixmap;
    cachedMsaRect_ = msaRect;
}
