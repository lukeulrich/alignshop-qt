/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include "SelectMsaTool.h"
#include "../painting/IRenderEngine.h"
#include "../widgets/AbstractMsaView.h"
#include "../../core/global.h"
#include "../../core/macros.h"
#include "../../core/util/ClosedIntRange.h"

/**
  * SelectionExtenderPrivate is a helper friend class of SelectMsaTool that provides a user-friendly visual "handles" to
  * rapidly and easily adjust a predefined selection.
  *
  * There are 8 handle positions and these are positioned just outside the selection - one at each corner and one in the
  * middle of each side of the rectangular selection. By grabbing these handles with the mouse it is possible to extend
  * or shrink the selection using the mouse.
  *
  * Currently, each handle is represented by a square of constant size (regardless of zoom) and is positioned by a fixed
  * number of pixels from its relative point on the rectangular selection.
  *
  * ISSUES:
  * o When selection is adjacent to border of alignment view, some of the handles are not visible and thus it is not
  *   possible to adjust the selection in all directions.
  */
class SelectionExtenderPrivate : public QObject
{
public:
    enum Handle
    {
        TopHandle = 0,
        TopRightHandle,
        RightHandle,
        BottomRightHandle,
        BottomHandle,
        BottomLeftHandle,
        LeftHandle,
        TopLeftHandle
    };

    // -------------------------------------------------------------------------------------------------
    // Constructor
    SelectionExtenderPrivate(SelectMsaTool *selectMsaTool);

    // -------------------------------------------------------------------------------------------------
    // Public methods
    bool isPointOverControl(const QPoint &point) const;         //!< Convenience method for checking if point is over a handle or side
    void viewportMousePressEvent(QMouseEvent *mouseEvent);
    void viewportMouseMoveEvent(QMouseEvent *mouseEvent);
    void viewportMouseReleaseEvent(QMouseEvent *mouseEvent);
    void viewportPaint(IRenderEngine *renderEngine, QPainter *painter);


protected:
    // -------------------------------------------------------------------------------------------------
    // Protected methods
    //! Returns the corresponding QRectF in view space relative to viewRectangle for handle
    QRectF handleRect(const QRectF &viewRectangle, Handle handle) const;
    bool isPointOverHandle(const QPoint &point) const;          //!< Returns true if any of the 8 handles contains point; false otherwise
    //! Returns true if point is over the "grabbable" area of any of the four sides; false otherwise; if true, then side is set to the side that point is over
    bool isPointOverSide(const QPoint &point, Side &side) const;
    bool isPointOverSide(const QPoint &point) const;            //!< Similar to the above operation, but does not require a side argument
    QRectF sideRect(const QRectF &viewRectangle, const Side &side) const;


private:
    // -------------------------------------------------------------------------------------------------
    // Private members
    static const int kHandleWidth = 8;              //!< Size of each side of the square handle in pixels
    static const int kHandleRectSpacing = 3;        //!< Pixels between selection rectangle and start of handle
    static const int kSideGrabSize = 6;             //!< Number of pixels to function as the grabbable area

    SelectMsaTool *selectMsaTool_;                  //!< Owning instance and parent selection tool
    bool active_;                                   //!< Flag indicating if a handle is currently being dragged
};


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param selectMsaTool [SelectMsaTool *]
  */
SelectionExtenderPrivate::SelectionExtenderPrivate(SelectMsaTool *selectMsaTool)
    : QObject(selectMsaTool),
      selectMsaTool_(selectMsaTool),
      active_(false)
{
    ASSERT(selectMsaTool_ != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param point [const QPoint &]
  * @returns bool
  */
bool SelectionExtenderPrivate::isPointOverControl(const QPoint &point) const
{
    return isPointOverSide(point) || isPointOverHandle(point);
}

/**
  * Grabbing is only done by using the left mouse button. When a handle is selected, the corresponding anchor point for
  * this operation is the opposite-most point in the selection rectangle. For example, this would be the BottomLeft
  * Handle if the TopRight handle is selected and vice versa.
  *
  * @param mouseEvent [QMouseEvent *]
  */
void SelectionExtenderPrivate::viewportMousePressEvent(QMouseEvent *mouseEvent)
{
    // By default, we do not handle this mouseEvent
    mouseEvent->ignore();
    if (mouseEvent->button() != Qt::LeftButton)
        return;

    // Only work with a non-null msa selection. If there is no selection, it is impossible to extend the selection using
    // a handle.
    PosiRect selection = selectMsaTool_->msaView_->selection().normalized();
    if (selection.isNull())
        return;

    // Did this mouse click occur on one of our handle rects?
    QRectF viewSelectionRect = selectMsaTool_->msaView_->pointRectMapper()->msaRectToViewRect(selection);
    for (int i=0; i<8; ++i)
    {
        QRectF rect = handleRect(viewSelectionRect, static_cast<Handle>(i));
        if (!rect.contains(mouseEvent->pos()))
            continue;

        // Yes. Activate the handle and initiate the selection process
        active_ = true;
        Handle activeHandle = static_cast<Handle>(i);
        // Since our handles are outside the rectangular selection, it is necessary to adjust the mouse hot spot. This
        // varies depending on the exact handle but in general is the difference between the center of the closest msa
        // point to the handle and current mouse position.
        QPoint centerOfSelectionStop;

        switch (activeHandle)
        {
        case TopHandle:
            selectMsaTool_->msaStartAnchorPoint_ = selection.bottomLeft();
            selectMsaTool_->setSelectionAxis(Ag::VerticalAxis);
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(QPoint(selection.center().x(), selection.top())).center().toPoint();
            break;
        case TopRightHandle:
            selectMsaTool_->msaStartAnchorPoint_ = selection.bottomLeft();
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(selection.topRight()).center().toPoint();
            break;
        case RightHandle:
            selectMsaTool_->msaStartAnchorPoint_ = selection.topLeft();
            selectMsaTool_->setSelectionAxis(Ag::HorizontalAxis);
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(QPoint(selection.right(), selection.center().y())).center().toPoint();
            break;
        case BottomRightHandle:
            selectMsaTool_->msaStartAnchorPoint_ = selection.topLeft();
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(selection.bottomRight()).center().toPoint();
            break;
        case BottomHandle:
            selectMsaTool_->msaStartAnchorPoint_ = selection.topLeft();
            selectMsaTool_->setSelectionAxis(Ag::VerticalAxis);
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(QPoint(selection.center().x(), selection.bottom())).center().toPoint();
            break;
        case BottomLeftHandle:
            selectMsaTool_->msaStartAnchorPoint_ = selection.topRight();
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(selection.bottomLeft()).center().toPoint();
            break;
        case LeftHandle:
            selectMsaTool_->msaStartAnchorPoint_ = selection.topRight();
            selectMsaTool_->setSelectionAxis(Ag::HorizontalAxis);
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(QPoint(selection.left(), selection.center().y())).center().toPoint();
            break;
        case TopLeftHandle:
            selectMsaTool_->msaStartAnchorPoint_ = selection.bottomRight();
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(selection.topLeft()).center().toPoint();
            break;
        }

        selectMsaTool_->selectionIsActive_ = true;
        selectMsaTool_->msaView_->setMouseHotSpotOffset(centerOfSelectionStop - mouseEvent->pos());

        // Inform downstream components that this mouse event has been processed
        mouseEvent->accept();

        // It is technically possible for the mouse to overlap two handles simultaneously (ie. view is extremely zoomed
        // out), yet only process the first contained handle.
        return;
    }

    // -----------------------
    // Now check for a side :)
    Side side = TopSide;
    if (isPointOverSide(mouseEvent->pos(), side))
    {
        active_ = true;
        QPoint centerOfSelectionStop;

        QPoint msaPoint = selectMsaTool_->msaView_->pointRectMapper()->viewPointToMsaPoint(mouseEvent->pos());

        switch (side)
        {
        case TopSide:
            selectMsaTool_->msaStartAnchorPoint_ = selection.bottomLeft();
            selectMsaTool_->setSelectionAxis(Ag::VerticalAxis);
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(QPoint(msaPoint.x(), selection.top())).center().toPoint();
            break;
        case RightSide:
            selectMsaTool_->msaStartAnchorPoint_ = selection.topLeft();
            selectMsaTool_->setSelectionAxis(Ag::HorizontalAxis);
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(QPoint(selection.right(), msaPoint.y())).center().toPoint();
            break;
        case BottomSide:
            selectMsaTool_->msaStartAnchorPoint_ = selection.topLeft();
            selectMsaTool_->setSelectionAxis(Ag::VerticalAxis);
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(QPoint(msaPoint.x(), selection.bottom())).center().toPoint();
            break;
        case LeftSide:
            selectMsaTool_->msaStartAnchorPoint_ = selection.topRight();
            selectMsaTool_->setSelectionAxis(Ag::HorizontalAxis);
            centerOfSelectionStop = selectMsaTool_->msaView_->pointRectMapper()->msaPointToViewRect(QPoint(selection.left(), msaPoint.y())).center().toPoint();
            break;
        }

        selectMsaTool_->selectionIsActive_ = true;
        selectMsaTool_->msaView_->setMouseHotSpotOffset(centerOfSelectionStop - mouseEvent->pos());
        mouseEvent->accept();

        return;
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void SelectionExtenderPrivate::viewportMouseMoveEvent(QMouseEvent *mouseEvent)
{
    if (active_)
        return;

    if (isPointOverHandle(mouseEvent->pos()) ||
        isPointOverSide(mouseEvent->pos()))
    {
        selectMsaTool_->msaView_->hideMouseCursorPoint();
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void SelectionExtenderPrivate::viewportMouseReleaseEvent(QMouseEvent *mouseEvent)
{
    Q_UNUSED(mouseEvent);

    active_ = false;
    selectMsaTool_->msaView_->setMouseHotSpotOffset(QPoint());
    selectMsaTool_->setSelectionAxis(Ag::HorizontalVerticalAxis);

    if (isPointOverHandle(mouseEvent->pos()) ||
        isPointOverSide(mouseEvent->pos()))
    {
        selectMsaTool_->msaView_->hideMouseCursorPoint();
    }

    // Special case: user makes a selection and then releases the mouse without any movement. At this point, the
    // selection has completed and seleciton modifying handles should appear; however, without triggering an update
    // nothing will happen. This update request ensures that they are drawn now rather than waiting for a mouse move
    // event to request an update.
    selectMsaTool_->msaView_->viewport()->update();
}

/**
  * @param renderEngin [IRenderEngine *]
  * @param painter [QPainter *]
  */
void SelectionExtenderPrivate::viewportPaint(IRenderEngine *renderEngine, QPainter *painter)
{
    // Render nothing if a non-handle selection or slide is in progress
    if (active_ ||
        // This next condition should really be: (!active_ && selectionIsActive_); however,
        // if this condition is tested, then !active is implied because of the order of tests
        selectMsaTool_->selectionIsActive_ ||
        selectMsaTool_->slideIsActive_ ||
        selectMsaTool_->msaView_->selection().isNull())
    {
        return;
    }

    QRectF viewSelectionRect =
            selectMsaTool_->msaView_->pointRectMapper()->msaRectToViewRect(selectMsaTool_->msaView_->selection());
    QPoint mousePos = selectMsaTool_->msaView_->viewport()->mapFromGlobal(QCursor::pos());

    // Context: Non-empty selection that is not actively being horizontally slid or created without dragging a handle

    // Check if we are over a side and increase the line width by 1 pixel if hovering over a line
    renderEngine->setLineWidth(renderEngine->lineWidth() + 1);
    {
        Side side = TopSide;
        if (isPointOverSide(mousePos, side))
            renderEngine->outlineSideInside(viewSelectionRect.toRect(), side, Qt::green, painter);
    }

    // Now render the handles
    renderEngine->setLineWidth(1.);

    // When zoomed out really far it is possible for the mouse position to be contained in multiple handle rectangles.
    // To prevent visually displaying this, simply highlight the first one that is contained.
    bool foundOneForMousePosition = false;

    // Draw all 8 handles
    for (int i=0; i<8; ++i)
    {
        QRect rect = handleRect(viewSelectionRect, static_cast<Handle>(i)).toRect();
        if (rect.isNull())
            continue;

        if (foundOneForMousePosition || !rect.contains(mousePos))
        {
            renderEngine->fillRect(rect, Qt::black, painter);
            renderEngine->outlineRectInside(rect, Qt::white, painter);
        }
        else
        {
            renderEngine->fillRect(rect, Qt::green, painter);
            renderEngine->outlineRectInside(rect, Qt::black, painter);
            foundOneForMousePosition = true;
        }
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param viewRectangle [const QRectF &]
  * @param handle [Handle]
  */
QRectF SelectionExtenderPrivate::handleRect(const QRectF &viewRectangle, Handle handle) const
{
    if (viewRectangle.isNull())
        return QRectF();

    double halfHandleWidth = kHandleWidth / 2.;
    double halfViewRectWidth = viewRectangle.width() / 2.;
    double halfViewRectHeight = viewRectangle.height() / 2.;

    QRectF handleRect(0, 0, kHandleWidth, kHandleWidth);

    switch (handle)
    {
    case TopHandle:
        handleRect.moveTop(viewRectangle.top() - kHandleWidth - kHandleRectSpacing);
        handleRect.moveLeft(viewRectangle.left() + halfViewRectWidth - halfHandleWidth);
        break;
    case TopRightHandle:
        handleRect.moveTop(viewRectangle.top() - kHandleWidth - kHandleRectSpacing);
        handleRect.moveRight(viewRectangle.right() + kHandleWidth + kHandleRectSpacing);
        break;
    case RightHandle:
        handleRect.moveTop(viewRectangle.top() + halfViewRectHeight - halfHandleWidth);
        handleRect.moveRight(viewRectangle.right() + kHandleWidth + kHandleRectSpacing);
        break;
    case BottomRightHandle:
        handleRect.moveBottom(viewRectangle.bottom() + kHandleWidth + kHandleRectSpacing);
        handleRect.moveRight(viewRectangle.right() + kHandleWidth + kHandleRectSpacing);
        break;
    case BottomHandle:
        handleRect.moveBottom(viewRectangle.bottom() + kHandleWidth + kHandleRectSpacing);
        handleRect.moveLeft(viewRectangle.left() + halfViewRectWidth - halfHandleWidth);
        break;
    case BottomLeftHandle:
        handleRect.moveBottom(viewRectangle.bottom() + kHandleWidth + kHandleRectSpacing);
        handleRect.moveLeft(viewRectangle.left() - kHandleWidth - kHandleRectSpacing);
        break;
    case LeftHandle:
        handleRect.moveTop(viewRectangle.top() + halfViewRectHeight - halfHandleWidth);
        handleRect.moveLeft(viewRectangle.left() - kHandleWidth - kHandleRectSpacing);
        break;
    case TopLeftHandle:
        handleRect.moveTop(viewRectangle.top() - kHandleWidth - kHandleRectSpacing);
        handleRect.moveLeft(viewRectangle.left() - kHandleWidth - kHandleRectSpacing);
        break;


/*
    // For placing the handles along the rectangle edges
    case TopHandle:
        handleRect.moveTop(viewRectangle.top() - halfHandleWidth);
        handleRect.moveLeft(viewRectangle.left() + halfViewRectWidth - halfHandleWidth);
        break;
    case TopRightHandle:
        handleRect.moveTop(viewRectangle.top() - halfHandleWidth);
        handleRect.moveRight(viewRectangle.right() + halfHandleWidth);
        break;
    case RightHandle:
        handleRect.moveTop(viewRectangle.top() + halfViewRectHeight - halfHandleWidth);
        handleRect.moveRight(viewRectangle.right() + halfHandleWidth);
        break;
    case BottomRightHandle:
        handleRect.moveBottom(viewRectangle.bottom() + halfHandleWidth);
        handleRect.moveRight(viewRectangle.right() + halfHandleWidth);
        break;
    case BottomHandle:
        handleRect.moveBottom(viewRectangle.bottom() + halfHandleWidth);
        handleRect.moveLeft(viewRectangle.left() + halfViewRectWidth - halfHandleWidth);
        break;
    case BottomLeftHandle:
        handleRect.moveBottom(viewRectangle.bottom() + halfHandleWidth);
        handleRect.moveLeft(viewRectangle.left() - halfHandleWidth);
        break;
    case LeftHandle:
        handleRect.moveTop(viewRectangle.top() + halfViewRectHeight - halfHandleWidth);
        handleRect.moveLeft(viewRectangle.left() - halfHandleWidth);
        break;
    case TopLeftHandle:
        handleRect.moveTop(viewRectangle.top() - halfHandleWidth);
        handleRect.moveLeft(viewRectangle.left() - halfHandleWidth);
        break;
*/
    }

    return handleRect;
}

/**
  * @param point [const QPoint &]
  * @returns bool
  */
bool SelectionExtenderPrivate::isPointOverHandle(const QPoint &point) const
{
    PosiRect selection = selectMsaTool_->msaView_->selection().normalized();
    if (selection.isNull())
        return false;

    QRectF viewSelectionRect = selectMsaTool_->msaView_->pointRectMapper()->msaRectToViewRect(selection);
    for (int i=0; i<8; ++i)
    {
        QRectF rect = handleRect(viewSelectionRect, static_cast<Handle>(i));
        if (rect.contains(point))
            return true;
    }

    return false;
}

/**
  * @param point [const QPoint &]
  * @param side [Side &]
  * @returns bool
  */
bool SelectionExtenderPrivate::isPointOverSide(const QPoint &point, Side &side) const
{
    PosiRect selection = selectMsaTool_->msaView_->selection().normalized();
    if (selection.isNull())
        return false;

    QRectF viewSelectionRect = selectMsaTool_->msaView_->pointRectMapper()->msaRectToViewRect(selection);
    for (int i=0; i<4; ++i)
    {
        Side currentSide = static_cast<Side>(i);
        QRectF rect = sideRect(viewSelectionRect, currentSide);
        if (rect.contains(point))
        {
            side = currentSide;
            return true;
        }
    }

    return false;
}

/**
  * @param point [const QPoint &]
  * @param side [Side &]
  * @returns bool
  */
bool SelectionExtenderPrivate::isPointOverSide(const QPoint &point) const
{
    Side dummy;
    return isPointOverSide(point, dummy);
}

/**
  * @param side [const Side &]
  * @returns QRectF
  */
QRectF SelectionExtenderPrivate::sideRect(const QRectF &viewRectangle, const Side &side) const
{
    if (viewRectangle.isNull())
        return QRectF();

    QRectF sideRectangle = viewRectangle;

    double halfGrabSize = kSideGrabSize / 2.;

    switch (side)
    {
    case TopSide:
        sideRectangle.setHeight(kSideGrabSize);
        sideRectangle.moveTop(viewRectangle.top() - halfGrabSize);
        break;
    case RightSide:
        sideRectangle.setWidth(kSideGrabSize);
        sideRectangle.moveLeft(viewRectangle.right() - halfGrabSize);
        break;
    case BottomSide:
        sideRectangle.setHeight(kSideGrabSize);
        sideRectangle.moveTop(viewRectangle.bottom() - halfGrabSize);
        break;
    case LeftSide:
        sideRectangle.setWidth(kSideGrabSize);
        sideRectangle.moveLeft(viewRectangle.left() - halfGrabSize);
        break;
    }

    return sideRectangle;
}
