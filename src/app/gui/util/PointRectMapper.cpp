/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QScrollBar>
#include <cmath>

#include "PointRectMapper.h"
#include "../widgets/AbstractMsaView.h"
#include "../../core/misc.h"

#ifdef QT_DEBUG
#include "../../core/ObservableMsa.h"
#endif

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param abstractMsaView [AbstractMsaView *]
  * @param resolution [int]
  */
PointRectMapper::PointRectMapper(AbstractMsaView *abstractMsaView)
    : abstractMsaView_(abstractMsaView)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns AbstractMsaView *
  */
AbstractMsaView *PointRectMapper::abstractMsaView() const
{
    return abstractMsaView_;
}

/**
  * @param abstractMsaView [AbstractMsaView *]
  */
void PointRectMapper::setAbstractMsaView(AbstractMsaView *abstractMsaView)
{
    abstractMsaView_ = abstractMsaView;
}

/**
  * @param canvasPoint [const QPoint &]
  * @returns QPoint
  */
QPoint PointRectMapper::canvasPointToMsaPoint(const QPointF &canvasPointF) const
{
    return ::floorPoint(canvasPointToMsaPointF(canvasPointF));
}

/**
  * @param canvasPoint [const QPoint &]
  * @returns QPoint
  */
QPointF PointRectMapper::canvasPointToMsaPointF(const QPointF &canvasPointF) const
{
    if (!abstractMsaView_ || abstractMsaView_->canvasSizeF().isNull())
        return QPointF();

    ASSERT_X(abstractMsaView_->msa_, "msa has not been defined");
    ASSERT_X(canvasPointF.x() >= 0 && canvasPointF.x() < abstractMsaView_->canvasSizeF().width(), "canvasPointF.x() out of range");
    ASSERT_X(canvasPointF.y() >= 0 && canvasPointF.y() < abstractMsaView_->canvasSizeF().height(), "canvasPointF.y() out of range");

    // The value is within the proper bounds, now perform the calculation
    return unboundedCanvasPointToMsaPointF(canvasPointF);
}

/**
  * @param canvasRect [const Rect &]
  * @returns PosiRect
  */
PosiRect PointRectMapper::canvasRectToMsaRect(const Rect &canvasRect) const
{
    return canvasRectFToMsaRect(canvasRect.toQRectF());

    if (!abstractMsaView_)
        return PosiRect();

    Rect normalizedRect = canvasRect.normalized();
    ASSERT_X(normalizedRect.width() > 0 && normalizedRect.height() > 0, "normalized rect must have a non-zero width and height");
    ASSERT_X(Rect(QPoint(0, 0), abstractMsaView_->canvasSize()).contains(canvasRect.normalized()), "rect outside of canvas boundaries");



    return PosiRect(canvasPointToMsaPoint(normalizedRect.topLeft()),
                   canvasPointToMsaPoint(QPointF(normalizedRect.right() - 1,
                                                 normalizedRect.bottom() - 1)));
}

/**
  * If the normalized canvasRectF has a zero width or height a default Rect is returned.
  *
  * @param canvasRect [const QRectF &]
  * @returns PosiRect
  */
PosiRect PointRectMapper::canvasRectFToMsaRect(const QRectF &canvasRectF) const
{
    if (!abstractMsaView_)
        return Rect();

    QRectF normalizedRect = canvasRectF.normalized();
    if (normalizedRect.width() == 0 || normalizedRect.height() == 0)
        return Rect();

    ASSERT_X(QRectF(QPointF(0, 0), abstractMsaView_->canvasSizeF()).contains(normalizedRect), "rect outside of canvas boundaries");

    // The above statements ensure that we are within the proper canvas boundaries
    return PosiRect(unboundedCanvasPointToMsaPoint(normalizedRect.topLeft()),
                   QPoint(ceil(normalizedRect.right() / abstractMsaView_->charWidth()),
                          ceil(normalizedRect.bottom() / abstractMsaView_->charHeight())));
}

/**
  * msaPoint may either be fractional or integral.
  *
  * @param msaPoint  [const QPointF &]
  * @returns QPointF
  */
QPointF PointRectMapper::msaPointToCanvasPoint(const QPointF &msaPointF) const
{
    if (!abstractMsaView_)
        return QPointF();

    ASSERT_X(!msaPointF.isNull(), "msaPoint must not be null");
    ASSERT_X(msaPointF.x() >= 1. && msaPointF.y() >= 1., "msaPoint out of range");

#ifdef QT_DEBUG
#ifndef TESTING
    // Print to console if point is outside the permissible ObservableMsa boundaries
    if (!abstractMsaView_->msa_)
        qWarning("AbstractMsaView::msaPointToCanvasPoint - msa not defined");
    else if (msaPointF.x() >= abstractMsaView_->msa_->length()+1 || msaPointF.y() >= abstractMsaView_->msa_->subseqCount()+1)
        qWarning("AbstractMsaView::msaPointToCanvasPoint - msaPointF (%.1f, %.1f) outside msa boundaries (%d, %d)", msaPointF.x(), msaPointF.y(), abstractMsaView_->msa_->length(), abstractMsaView_->msa_->subseqCount());
#endif
#endif

    return QPointF((msaPointF.x()-1.) * abstractMsaView_->charWidth(),
                   (msaPointF.y()-1.) * abstractMsaView_->charHeight());
}


/**
  * @param msaPoint [const QPoint &]
  * @returns QRectF
  */
QRectF PointRectMapper::msaPointToCanvasRect(const QPoint &msaPoint) const
{
    if (!abstractMsaView_)
        return QRectF();

    ASSERT_X(!msaPoint.isNull(), "msaPoint must not be null");
    ASSERT_X(msaPoint.x() > 0 && msaPoint.y() > 0, "msaPoint out of range");

#ifdef QT_DEBUG
#ifndef TESTING
    // Print to console if point is outside the permissible ObservableMsa boundaries
    if (!abstractMsaView_->msa_)
        qWarning("AbstractMsaView::msaPointToCanvasPoint - msa not defined");
    else if (msaPoint.x() > abstractMsaView_->msa_->length() || msaPoint.y() > abstractMsaView_->msa_->subseqCount())
        qWarning("AbstractMsaView::msaPointToCanvasPoint - msaPoint (%d, %d) outside msa boundaries (%d, %d)", msaPoint.x(), msaPoint.y(), abstractMsaView_->msa_->length(), abstractMsaView_->msa_->subseqCount());
#endif
#endif

    return QRectF(msaPointToCanvasPoint(msaPoint),
                  QSizeF(abstractMsaView_->charWidth(), abstractMsaView_->charHeight()));
}

/**
  * PosiRect must have x and y greater than 0 and x + width > 0 and y + height > 0. No other checking is performed to
  * ensure that PosiRect is a valid rectangle within the alignment.
  *
  * All rectangular coordinates in any quadrant are converted into a normalized representation and a valid rectangle is
  * returned.
  *
  * @param msaRect [const PosiRect &]
  * @returns QRectF
  */
QRectF PointRectMapper::msaRectToCanvasRect(const Rect &msaRect) const
{
    if (!abstractMsaView_)
        return QRectF();

    // Custom normalization with respect to 1-based ObservableMsa coordinates
    Rect normalizedRect = msaRect.normalized();
    ASSERT_X(normalizedRect.left() > 0 && normalizedRect.top() > 0, "rect left and top must both be greater than zero");
    ASSERT_X(normalizedRect.right() > 0 && normalizedRect.bottom() > 0, "rect right and bottom must be greater than zero");

#ifdef QT_DEBUG
#ifndef TESTING
    // Print to console if point is outside the permissible ObservableMsa boundaries
    if (!abstractMsaView_->msa_)
        qWarning("AbstractMsaView::msaRectToCanvasRect - msa not defined");
    else if (normalizedRect.left() > abstractMsaView_->msa_->length()
             || normalizedRect.right() > abstractMsaView_->msa_->length()
             || normalizedRect.top() > abstractMsaView_->msa_->subseqCount()
             || normalizedRect.bottom() > abstractMsaView_->msa_->subseqCount())
    {
        qWarning("AbstractMsaView::msaRectToCanvasRect - rect [(%d, %d) -> (%d, %d)] outside msa boundaries (%d, %d)",
                 normalizedRect.left(), normalizedRect.top(), normalizedRect.right(), normalizedRect.bottom(), abstractMsaView_->msa_->length(), abstractMsaView_->msa_->subseqCount());
    }
#endif
#endif

    return QRectF(msaPointToCanvasPoint(normalizedRect.topLeft()),
                  QSizeF((normalizedRect.width()+1) * abstractMsaView_->charWidth(),
                         (normalizedRect.height()+1) * abstractMsaView_->charHeight()));
}

/**
  * The view space is simply a translated representation of the canvas space. Therefore, conversion simply requires
  * a translation operation.
  *
  * @param canvasPoint [const QPointF &]
  * @returns QPointF
  */
QPointF PointRectMapper::canvasPointToViewPoint(const QPointF &canvasPointF) const
{
    if (!abstractMsaView_)
        return QPointF();

    ASSERT_X(abstractMsaView_->msa_, "msa has not been defined");
    ASSERT_X(canvasPointF.x() >= 0 && canvasPointF.x() < abstractMsaView_->canvasSizeF().width(), "canvasPointF.x() out of range");
    ASSERT_X(canvasPointF.y() >= 0 && canvasPointF.y() < abstractMsaView_->canvasSizeF().height(), "canvasPointF.y() out of range");

    QPointF viewOrigin(abstractMsaView_->horizontalScrollBar()->value(),
                       abstractMsaView_->verticalScrollBar()->value());

    return canvasPointF - viewOrigin;
}

/**
  * The argument, canvasRectF, must be contained in the actual canvas; however, its resulting point in view space does
  * not necessarily need to be within the visible viewport boundaries.
  *
  * @param canvasRect [const QRectF &]
  * @returns QRectF
  */
QRectF PointRectMapper::canvasRectToViewRect(const QRectF &canvasRectF) const
{
    if (!abstractMsaView_)
        return QRectF();

    QRectF normalizedRect = canvasRectF.normalized();
    ASSERT_X(normalizedRect.width() > 0 && normalizedRect.height() > 0, "normalized rect must have a non-zero width and height");
    ASSERT_X(QRectF(QPointF(0, 0), abstractMsaView_->canvasSizeF()).contains(normalizedRect), "canvasRectF outside of canvas boundaries");

    return QRectF(canvasPointToViewPoint(normalizedRect.topLeft()),
                  canvasRectF.size());
}

/**
  * Because the view space is completely unbounded, it is possible for a viewpoint to not have a directly corresponding
  * value in the canvas space. In these cases, the closest canvas point is returned. Because no individual canvas point
  * can exist on the bottom/right-most edges of the canvas space, the actual canvas value returned for viewpoints
  * greater than the canvas size will be the canvassize - .0001.
  *
  * @param viewPointF [const QPointF &]
  * @returns QPointF
  * @see viewRectToCanvasRect()
  */
QPointF PointRectMapper::viewPointToCanvasPoint(const QPointF &viewPointF) const
{
    if (!abstractMsaView_)
        return QPointF();

    QSizeF canvasSizeF = abstractMsaView_->canvasSizeF();

    return QPointF(qBound(0., viewPointF.x() + abstractMsaView_->horizontalScrollBar()->value(), canvasSizeF.width()-.0001),
                   qBound(0., viewPointF.y() + abstractMsaView_->verticalScrollBar()->value(), canvasSizeF.height()-.0001));
}

/**
  * Similar logic applies here regarding the viewpoint -> canvas mapping as described in viewPointToCanvasPoint.
  *
  * @param viewRectF [const QRectF &]
  * @returns QRectF
  * @see viewPointToCanvasPoint()
  */
QRectF PointRectMapper::viewRectToCanvasRect(const QRectF &viewRectF) const
{
    if (!abstractMsaView_)
        return QRectF();

    QRectF normalizedRect = viewRectF.normalized();
    return QRectF(viewPointToCanvasPoint(normalizedRect.topLeft()),
                  viewPointToCanvasPoint(normalizedRect.bottomRight()));
}

/**
  * @param viewPointF [const QPointF &]
  * @returns QPoint
  */
QPoint PointRectMapper::viewPointToMsaPoint(const QPointF &viewPointF) const
{
    return ::floorPoint(viewPointToMsaPointF(viewPointF));
}

/**
  * @param viewPointF [const QPointF &]
  * @returns QPointF
  */
QPointF PointRectMapper::viewPointToMsaPointF(const QPointF &viewPointF) const
{
    return canvasPointToMsaPointF(viewPointToCanvasPoint(viewPointF));
}

/**
  * @param msaPointF [const QPointF &]
  * @returns QPointF
  */
QPointF PointRectMapper::msaPointToViewPoint(const QPointF &msaPointF) const
{
    return canvasPointToViewPoint(msaPointToCanvasPoint(msaPointF));
}

/**
  * @param msaPoint [const QPoint &]
  * @returns QPointF
  */
QRectF PointRectMapper::msaPointToViewRect(const QPoint &msaPoint) const
{
    return canvasRectToViewRect(msaPointToCanvasRect(msaPoint));
}

/**
  * @param viewRectF [const QRectF &]
  * @returns Rect
  */
PosiRect PointRectMapper::viewRectToMsaRect(const QRectF &viewRectF) const
{
    return canvasRectFToMsaRect(viewRectToCanvasRect(viewRectF));
}

/**
  * @param msaRect [const PosiRect &]
  * @returns QRectF
  */
QRectF PointRectMapper::msaRectToViewRect(const PosiRect &msaRect) const
{
    return canvasRectToViewRect(msaRectToCanvasRect(msaRect));
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param canvasPointF
  * @returns QPointF
  */
QPointF PointRectMapper::unboundedCanvasPointToMsaPointF(const QPointF &canvasPointF) const
{
    ASSERT(abstractMsaView_);

    // Must add 1 to both the x and y derived values to make it 1-based
    return QPointF(1. + canvasPointF.x() / abstractMsaView_->charWidth(),
                   1. + canvasPointF.y() / abstractMsaView_->charHeight());
}

/**
  * @param canvasPointF
  * @returns QPoint
  */
QPoint PointRectMapper::unboundedCanvasPointToMsaPoint(const QPointF &canvasPointF) const
{
    return ::floorPoint(unboundedCanvasPointToMsaPointF(canvasPointF));
}
