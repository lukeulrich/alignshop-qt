/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QRectF>

#include <QtGui/QScrollBar>

#include "GapMsaTool.h"
#include "MsaToolTypes.h"
#include "../widgets/AbstractMsaView.h"
#include "../../core/ObservableMsa.h"
#include "../painting/IRenderEngine.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QObject *]
  */
GapMsaTool::GapMsaTool(AbstractMsaView *msaView, QObject *parent)
    : AbstractMsaTool(msaView, parent),
      isActive_(false),
      controlPressed_(false),
      gapInsertionAnchorX_(0),
      pointRectMapper_(msaView)
{
    horizScrollBarVisible_ = false;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool GapMsaTool::isActive() const
{
    return isActive_;
}

/**
  * @returns type
  */
int GapMsaTool::type() const
{
    return Ag::kGapMsaTool;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void GapMsaTool::activate()
{
    msaView_->viewport()->setCursor(Qt::ArrowCursor);
    msaView_->hideMouseCursorPoint();

    AbstractMsaTool::activate();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param keyEvent [QKeyEvent *]
  */
void GapMsaTool::viewportKeyPressEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key())
    {
    case Qt::Key_Control:
        if (!keyEvent->isAutoRepeat())
            controlPressed_ = true;
        break;
    }
}

/**
  * @param keyEvent [QKeyEvent *]
  */
void GapMsaTool::viewportKeyReleaseEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key())
    {
    case Qt::Key_Control:
        if (!keyEvent->isAutoRepeat())
            controlPressed_ = false;
        break;
    }
}

/**
  */
void GapMsaTool::viewportMouseLeaveEvent()
{
    // To force the update and hiding of the gap insertion line
    msaView_->viewport()->update();
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void GapMsaTool::viewportMousePressEvent(QMouseEvent *mouseEvent)
{
    int mouseX = mouseEvent->pos().x();
    if (mouseEvent->button() == Qt::LeftButton)
    {
        isActive_ = true;
        gapInsertionAnchorX_ = gapInsertionColumn();
        // Note, as of this point, we have not inserted any gaps, but merely started the process, thus we set the end
        // to one less than the start - effectively making it "empty"
        emit gapColumnsInsertStarted(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ - 1), true);

        qreal originColumn = pointRectMapper_.viewPointToMsaPointF(QPoint(mouseX, 1)).x();

        gapStartViewX_ = gapPlotX(originColumn + .5);
        gapLastViewX_ = mouseX;
        gapsAdded_ = 0;

        // Temporarily diable the scroll bar during the move
        horizScrollBarVisible_ = msaView_->horizontalScrollBar()->isVisible();
        if (!horizScrollBarVisible_)
            msaView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // ----------------------------------------------------------------------------------------------------------
        // Special case: user presses left mouse button in area to the right of the alignment. Result: go ahead and
        //               create gap columns up to this point but keep the anchor in its original position (at the end
        //               of the alignment before the click).
        bool clickPointIsBeyondRightOfAlignment = mouseX > gapStartViewX_;
        if (clickPointIsBeyondRightOfAlignment)
        {
            qreal halfCharWidth = msaView_->charWidth() / 2.;
            int colsToAdd = (mouseX - gapStartViewX_ + halfCharWidth) / msaView_->charWidth();
            if (colsToAdd)
            {
                msaView_->msa()->insertGapColumns(gapInsertionAnchorX_, colsToAdd);
                gapsAdded_ += colsToAdd;
            }
        }

        msaView_->viewport()->update();
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void GapMsaTool::viewportMouseMoveEvent(QMouseEvent *mouseEvent)
{
    static int lastGapInsertionColumn = 0;

    if (isActive_)
    {
        // It is desirable that when the mouse has crossed over half of a block's width, to insert a gap at that
        // position. To achieve this and because of integer arithmetic,, it is necessary to add half of the block width
        // to the calculations of how many gaps to add. For instance, any X greater than 0 but less than the block width
        // when divided by the block width will return zero. Adding half of the block width to X addresses this problem.
        qreal halfCharWidth = msaView_->charWidth() / 2.;

        int mouseX = mouseEvent->pos().x();
        int deltaMouseX = mouseX - gapLastViewX_;
        if (deltaMouseX > 0)
        {
            // Moving in positive direction left of origin
            if (mouseX <= gapStartViewX_)
            {
                //          |
                //          |
                //  o-----> |
                //          |
                // ---------|++++++++
                int finalGapCount = (gapStartViewX_ - mouseX + halfCharWidth) / msaView_->charWidth();
                int colsToRemove = -gapsAdded_ - finalGapCount;

                if (colsToRemove)
                {
                    // Unfortunately, there is a coupling between removing the gap columns, rendering, and the x render
                    // shift that requires a specific order when updating the view parameters. More specifically, when
                    // removeGapColumns is called, it emits a direct signal with this information. The AbstractMsaView,
                    // connected to this signal, then immediately issues a queued viewport update and currently updates
                    // the margin widget geometries. The queued viewport update is no problem because this method will
                    // complete before the visual update is performed. But positioning of the margin widget geometries
                    // which depends upon the x render shift will occur immediately. Thus, it is important to first
                    // update the render x shift, then remove the gap columns, and then update the horizontal scroll bar
                    // if necessary.
                    if (!horizScrollBarVisible_)
                        // the scroll bar is off, but the number of gaps left of the origin is decreasing ==> adjust the
                        // msaView's shift amount accordingly - again to keep the right portion of the alignment fixed.
                        msaView_->setRenderXShift(-finalGapCount * msaView_->charWidth());

                    msaView_->msa()->removeGapColumns(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + colsToRemove - 1));
                    if (horizScrollBarVisible_)
                        // The horizontal scroll bar is visible ==> adjust the scrollbar such that the right half of the
                        // visible alignment stays in place.
                        msaView_->horizontalScrollBar()->setValue(msaView_->horizontalScrollBar()->value() - colsToRemove * msaView_->charWidth());

                    gapsAdded_ = -finalGapCount;

                    emit gapColumnsIntermediate(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + finalGapCount - 1), false);
                }
            }
            else
            {
                //          |
                //          |
                //          | o----->
                //          |
                // ---------|++++++++

                if (gapLastViewX_ <= gapStartViewX_)
                {
                    //          |
                    //          |
                    //        o-|--->
                    //          |
                    // ---------|++++++++

                    // User has dragged the mouse from left of origin to right of origin in fell swoop:
                    // 1) Remove the columns added to the left of origin
                    // 2) Add columns to right of origin
                    int colsToRemove = -gapsAdded_;
                    if (colsToRemove)
                    {
                        // Again, the order of execution here is vital for proper rendering. See the above for details.
                        if (!horizScrollBarVisible_)
                            // No scroll bar and we are back on the right side of the origin; reset the view shift factor
                            msaView_->setRenderXShift(0);

                        msaView_->msa()->removeGapColumns(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + colsToRemove - 1));
                        if (horizScrollBarVisible_)
                            msaView_->horizontalScrollBar()->setValue(msaView_->horizontalScrollBar()->value() - colsToRemove * msaView_->charWidth());

                        gapsAdded_ = 0;
                    }
                }

                int finalGapCount = (mouseX - gapStartViewX_ + halfCharWidth) / msaView_->charWidth();
                int colsToAdd = finalGapCount - gapsAdded_;
                if (colsToAdd)
                {
                    msaView_->msa()->insertGapColumns(gapInsertionAnchorX_, colsToAdd);
                    gapsAdded_ += colsToAdd;
                }

                emit gapColumnsIntermediate(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + finalGapCount - 1), true);
            }
        }
        else if (deltaMouseX < 0)
        {
            // Mouse is on right side of origin, simply removing columns
            if (mouseX >= gapStartViewX_)
            {
                //          |
                //          |
                //          |  <----o
                //          |
                // ---------|++++++++
                int finalGapCount = (mouseX - gapStartViewX_ + halfCharWidth) / msaView_->charWidth();
                int colsToRemove = gapsAdded_ - finalGapCount;
                if (colsToRemove)
                {
                    msaView_->msa()->removeGapColumns(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + colsToRemove - 1));
                    gapsAdded_ = finalGapCount;

                    emit gapColumnsIntermediate(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + finalGapCount - 1), true);
                }
            }
            else
            {
                //          |
                //          |
                //   <---o  |
                //          |
                // ---------|++++++++

                // Mouse is on left side of origin
                // Case 1: Was it previously on the right side of origin? If so, remove some of those columns
                if (gapLastViewX_ >= gapStartViewX_)
                {
                    //          |
                    //          |
                    //      <---|--o
                    //          |
                    // ---------|++++++++
                    int colsToRemove = gapsAdded_;
                    if (colsToRemove)
                    {
                        msaView_->msa()->removeGapColumns(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + colsToRemove - 1));
                        gapsAdded_ = 0;
                    }
                }

                int finalGapCount = (gapStartViewX_ - mouseX + halfCharWidth) / msaView_->charWidth();
                if (!horizScrollBarVisible_)
                    // The user is dragging the gap insertions left of the origin but there is not scroll bar. Emulate
                    // inserting gap columns to the left by translating the view rendering origin.
                    msaView_->setRenderXShift(-finalGapCount * msaView_->charWidth());

                int colsToAdd = finalGapCount - -gapsAdded_;
                if (colsToAdd)
                {
                    msaView_->msa()->insertGapColumns(gapInsertionAnchorX_, colsToAdd);
                    if (horizScrollBarVisible_)
                        msaView_->horizontalScrollBar()->setValue(msaView_->horizontalScrollBar()->value() + colsToAdd * msaView_->charWidth());

                    gapsAdded_ = -finalGapCount;
                }
                emit gapColumnsIntermediate(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + finalGapCount - 1), false);
            }
        }

        gapLastViewX_ = mouseEvent->pos().x();
    }
    else    // if (!isActive_)
    {
        int gapInsertionColumn = this->gapInsertionColumn();
        if (gapInsertionColumn != lastGapInsertionColumn)
        {
            emit gapInsertionColumnChanged(gapInsertionColumn);

            lastGapInsertionColumn = gapInsertionColumn;
        }
    }

    msaView_->viewport()->update();
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void GapMsaTool::viewportMouseReleaseEvent(QMouseEvent * /* mouseEvent */)
{
    if (isActive_)
    {
        isActive_ = false;
        emit gapColumnsInsertFinished(ClosedIntRange(gapInsertionAnchorX_, gapInsertionAnchorX_ + qAbs(gapsAdded_) - 1), gapsAdded_ >= 0);

        // Since we have finished, update the gap insertion column for the next insertion
        if (gapsAdded_ > 0)
            // If no gaps have been added, then the mouse position would be in the same place and thus this signal would
            // not need to be emitted. Or if the number of gaps added is negative, then insertion column also would not
            // have changed.
            emit gapInsertionColumnChanged(gapInsertionColumn());

        if (!horizScrollBarVisible_)
        {
            msaView_->setRenderXShift(0);
            msaView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }

        msaView_->updateMarginWidgetGeometries();
    }

    // To remove the boundary lines in case the user has not yet moved the mouse, refresh the screen
    msaView_->viewport()->update();
}

/**
  * @param renderEngine [IRenderEngine *]
  * @param painter [QPainter *]
  */
void GapMsaTool::viewportPaint(IRenderEngine *renderEngine, QPainter *painter)
{
    ASSERT_X(renderEngine != nullptr, "renderEngine must not be null");

    if (!isActive_ && !msaView_->viewport()->rect().contains(msaView_->viewport()->mapFromGlobal(QCursor::pos())))
        return;

    renderEngine->setLineWidth(2);
    int height = qMin(static_cast<qreal>(msaView_->viewport()->height() + .5),
                      renderEngine->abstractTextRenderer()->height() * msaView_->msa()->rowCount());

    if (isActive_)
    {
        // Note: we cannot use the gapInsertionAnchorX_ variable because it does not remain at the origin when adding
        // gaps to the left side of the origin. Thus, we have to calculate this everytime.
        qreal originColumn = pointRectMapper_.viewPointToMsaPointF(QPoint(gapStartViewX_, 1)).x();
        int originPlotX = gapPlotX(originColumn + .5);
        int boundaryPlotX = originPlotX + msaView_->charWidth() * gapsAdded_;

        // Because we want to draw a opacitous window, which is simply a filled rectangle bounded by two lines,
        // we first draw the filled rectangle with an opacity less than 1, then the origin and border lines.
        if (gapsAdded_)
        {
            // 1) Draw the filled rectangle
            renderEngine->fillRect(QRect(QPoint(originPlotX, 0), QPoint(boundaryPlotX, height)), QColor(0, 0, 0, 96), painter);

            // 2) The terminal boundary line
            renderEngine->drawLine(QPointF(boundaryPlotX, 0), QPointF(boundaryPlotX, height), Qt::black, painter);
        }

        // 3) The origin line
        renderEngine->drawLine(QPointF(originPlotX, 0), QPointF(originPlotX, height), QColor(96, 96, 96), painter);
    }
    else
    {
        int x = gapPlotX(gapInsertionColumn());
        renderEngine->drawLine(QPointF(x, 0), QPointF(x, height), Qt::black, painter);
    }
}

/**
  */
void GapMsaTool::viewportWindowDeactivate()
{
    isActive_ = false;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns int
  */
int GapMsaTool::gapInsertionColumn() const
{
    QPoint curMousePos = msaView_->viewport()->mapFromGlobal(QCursor::pos());
    return pointRectMapper_.viewPointToMsaPointF(curMousePos).x() + .5;
}

/**
  * @param msaColumn [int]
  * @returns qreal
  */
qreal GapMsaTool::gapPlotX(int msaColumn) const
{
    // Important! in the msaPointToViewPoint calculation, we add or subtract .0001 in case we are attempting to
    // add at the boundary of the alignment. This prevents an assertion from being thrown. Also, we really
    // only need the x value; however, the msaPointToViewPoint method takes a Point. Thus we supply a dummy
    // value of 1 for the y position
    qreal x = pointRectMapper_.msaPointToViewPoint(QPointF(qBound(1., static_cast<qreal>(msaColumn), msaColumn - .0001),
                                                            1.)).x();

    // Move the rendering position inside, if the gap is being inserted at either end of the alignment
    if (msaColumn == 1)
        x += 1.;
    else if (msaColumn == msaView_->msa()->length()+1)
        x -= 1.;

    return x;
}
