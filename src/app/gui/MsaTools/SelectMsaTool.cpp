/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QAction>
#include <QtGui/QScrollBar>

#include "HandMsaTool.h"
#include "MsaToolTypes.h"
#include "SelectMsaTool.h"
#include "SelectMsaTool_p.cpp"
#include "../widgets/AbstractMsaView.h"
#include "../../core/ObservableMsa.h"
#include "../Commands/Msa/SetSubseqStartCommand.h"
#include "../Commands/Msa/SetSubseqStopCommand.h"
#include "../Commands/Msa/CollapseMsaRectLeftCommand.h"
#include "../Commands/Msa/CollapseMsaRectRightCommand.h"


static const int kCollapseTolerance = 2;    // Must move at least this amount of pixels to cause a collapse

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QObject *]
  */
SelectMsaTool::SelectMsaTool(AbstractMsaView *msaView, QObject *parent)
    : AbstractMsaTool(msaView, parent),
      selectionIsActive_(false),
      slideIsActive_(false),
      collapseIsActive_(false),
      selectionAxis_(Ag::HorizontalVerticalAxis),
      pointRectMapper_(msaView),
      ctrlPressed_(false),
      handMsaTool_(nullptr),
      extendSequenceAction_(nullptr),
      trimSequenceAction_(nullptr),
      selectionExtender_(nullptr)
{
    // Set the msa selection timer timeout
    msaSelectionScrollTimer_.setInterval(50);
    connect(&msaSelectionScrollTimer_, SIGNAL(timeout()), SLOT(onMsaSelectionScrollTimeout()));

    extendSequenceAction_ = new QAction("Extend Sequence", this);
    extendSequenceAction_->setShortcut(QString("Ins"));
    extendSequenceAction_->setEnabled(false);
    connect(extendSequenceAction_, SIGNAL(triggered()), SLOT(onActionExtendSequence()));

    trimSequenceAction_ = new QAction("Trim Sequence", this);
    trimSequenceAction_->setShortcut(QString("Del"));
    trimSequenceAction_->setEnabled(false);
    connect(trimSequenceAction_, SIGNAL(triggered()), SLOT(onActionTrimSequence()));

    selectionExtender_ = new SelectionExtenderPrivate(this);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QAction *
  */
QAction *SelectMsaTool::extendSequenceAction() const
{
    return extendSequenceAction_;
}

/**
  * @returns bool
  */
bool SelectMsaTool::isActive() const
{
    return selectionIsActive_ || slideIsActive_ || collapseIsActive_;
}

/**
  * @param handMsaTool [HandMsaTool *]
  */
void SelectMsaTool::setHandMsaTool(HandMsaTool *handMsaTool)
{
    handMsaTool_ = handMsaTool;
}

/**
  * @param axis [const Ag::Axis]
  */
void SelectMsaTool::setSelectionAxis(const Ag::Axis axis)
{
    selectionAxis_ = axis;
}

/**
  * @returns QAction *
  */
QAction *SelectMsaTool::trimSequenceAction() const
{
    return trimSequenceAction_;
}

/**
  * @returns int
  */
int SelectMsaTool::type() const
{
    return Ag::kSelectMsaTool;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void SelectMsaTool::activate()
{
    updateMouseCursor();

    extendSequenceAction_->setEnabled(true);
    trimSequenceAction_->setEnabled(true);

    AbstractMsaTool::activate();
}

/**
  */
void SelectMsaTool::deactivate()
{
    finishSelectionSlide();
    collapseIsActive_ = false;

    msaView_->hideMouseCursorPoint();
    keysPressed_.clear();

    extendSequenceAction_->setEnabled(false);
    trimSequenceAction_->setEnabled(false);

    AbstractMsaTool::deactivate();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param keyEvent [QKeyEvent *]
  */
void SelectMsaTool::viewportKeyPressEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key())
    {
    case Qt::Key_Control:
        ctrlPressed_ = true;
        updateStop();
        break;
    case Qt::Key_Escape:
        if (!isActive() && msaView()->selection().isValid())
        {
            msaView_->clearSelection();
            emit selectionCleared();
        }
        updateMouseCursor();
        msaView_->viewport()->update();
        break;
    case Qt::Key_Space:
        if (handMsaTool_ != nullptr && !isActive() && !keyEvent->isAutoRepeat())
            // Second parameter indicates that this is a temporary switch to the hand tool
            msaView_->setCurrentMsaTool(handMsaTool_, true, keyEvent->key());
        break;

    default:
        if (!keyEvent->isAutoRepeat())
            keysPressed_ << keyEvent->key();
        break;
    }
}

/**
  * @param keyEvent [QKeyEvent *]
  */
void SelectMsaTool::viewportKeyReleaseEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key())
    {
    case Qt::Key_Control:
        if (!keyEvent->isAutoRepeat())
        {
            ctrlPressed_ = false;
            updateStop();
        }
        break;

    default:
        if (!keyEvent->isAutoRepeat())
            keysPressed_.remove(keyEvent->key());
        break;
    }
}

/**
  */
void SelectMsaTool::viewportMouseLeaveEvent()
{
    // Special case: in the select tool mode, mouse is off the viewport, the user selects all via Ctrl-A
    // then clicks the hand tool and then back to the select tool. The mouse cursor point is wrongly drawn
    // inside the selection. This condition prevents that from happening.
    msaView_->hideMouseCursorPoint();
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void SelectMsaTool::viewportMousePressEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        // Give precedence to the selection extender
        selectionExtender_->viewportMousePressEvent(mouseEvent);
        if (mouseEvent->isAccepted())
            return;

        if (msaView_->isMouseOverSelection())
        {
            slideIsActive_ = true;
            slideMsaAnchorPoint_ = msaView_->mouseCursorPoint();
            updateMouseCursor();
            emit slideStarted(msaView()->selection());
        }
        else
        {
            // Either a selection or slide is now in progress - prevent extend/trim sequences
            extendSequenceAction_->setEnabled(false);
            trimSequenceAction_->setEnabled(false);

            // Context: the user did not extend the selection via one of the handles, therefore, start a new selection
            viewAnchorPoint_ = msaView_->mouseHotSpot();
            selectionIsActive_ = true;

            Qt::KeyboardModifiers modifiers = mouseEvent->modifiers();
            bool shiftPressed = modifiers & Qt::ShiftModifier;

            // Special case: Alignment view does not have the focus, user presses control, and then presses the
            //               left mouse button on the view. Because the view was not focused when control was
            //               pressed, the class variable, ctrlPressed_ is not processed in the keyPressEvent method.
            //               Thus, also permit for setting the value from here.
            ctrlPressed_ = modifiers & Qt::ControlModifier;

            QPoint msaClickPoint = pointRectMapper_.viewPointToMsaPoint(viewAnchorPoint_);
            msaStartAnchorPoint_ = (!shiftPressed) ? msaClickPoint
                                                   : msaView_->selection().topLeft();
            QPoint start = msaStartAnchorPoint_;
            if (!shiftPressed)
                if (ctrlPressed_)
                    start.ry() = 1;

            QPoint stop = msaClickPoint;
            if (ctrlPressed_)
                stop.ry() = msaView_->msa()->rowCount();

            msaView_->setSelection(PosiRect(start, stop));
            msaView_->viewport()->update();
        }
    }
    else if (mouseEvent->button() == Qt::RightButton)
    {
        if (selectionIsActive_ || slideIsActive_ || !msaView_->isMouseOverSelection())
            return;

        PosiRect selection = msaView_->selection();
        if (selection.width() == 1)
            return;

        collapseIsActive_ = true;
        viewCollapseAnchorPoint_ = mouseEvent->pos();
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void SelectMsaTool::viewportMouseMoveEvent(QMouseEvent *mouseEvent)
{
    if (selectionIsActive_)
    {
        updateStop(msaView_->mouseHotSpot());

        // Automatically scroll if viewPoint is outside the viewport() rect bounds
        if (msaView_->viewport()->rect().contains(msaView_->mouseHotSpot()))
            msaSelectionScrollTimer_.stop();
        else if (!msaSelectionScrollTimer_.isActive())
            // Mouse has left the viewport area while selecting, initiate automatic scrolling
            msaSelectionScrollTimer_.start();
    }
    else if (slideIsActive_)
    {
        QPoint newSlideMsaPoint = msaView_->mouseCursorPoint();
        int msa_dx = newSlideMsaPoint.x() - slideMsaAnchorPoint_.x();
        if (msa_dx != 0)
        {
            // It is important that no normalization occurs here, otherwise, the slide may get pushed onto the stack
            // inadvertently.
            PosiRect mrect = msaView_->selection();
            int actual_delta = msaView_->msa()->slideRect(mrect, msa_dx);

            // Update the msa selection in accordance with how many residues were slid
            if (actual_delta != 0)
            {
                mrect.setLeft(mrect.left() + actual_delta);
                mrect.setRight(mrect.right() + actual_delta);
                msaView_->setSelection(mrect);
                slideMsaAnchorPoint_ = newSlideMsaPoint;
            }
        }

        msaView_->viewport()->update();
    }
    else if (collapseIsActive_)
    {
        int dx = mouseEvent->pos().x() - viewCollapseAnchorPoint_.x();
        if (dx > kCollapseTolerance)
        {
            PosiRect selection = msaView_->selection().normalized();
            if (msaView_->msa()->canCollapseRight(selection))
                msaView_->undoStack()->push(new CollapseMsaRectRightCommand(msaView_->msa(), selection));

            viewCollapseAnchorPoint_ = mouseEvent->pos();
        }
        else if (dx < -kCollapseTolerance)
        {
            PosiRect selection = msaView_->selection().normalized();
            if (msaView_->msa()->canCollapseLeft(selection))
                msaView_->undoStack()->push(new CollapseMsaRectLeftCommand(msaView_->msa(), selection));

            viewCollapseAnchorPoint_ = mouseEvent->pos();
        }
    }

    updateMouseCursor();

    // Finally, allow the selection extender to work with the mouse move data. Do this after updating the mouse cursor
    // so that it can override any mouse cursor settings done here.
    selectionExtender_->viewportMouseMoveEvent(mouseEvent);

    msaView_->viewport()->update();
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void SelectMsaTool::viewportMouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        finishSelectionSlide();
        updateMouseCursor();

        // Either a selection or slide is now finished - enable extend/trim sequences
        extendSequenceAction_->setEnabled(true);
        trimSequenceAction_->setEnabled(true);

        ASSERT(msaSelectionScrollTimer_.isActive() == false);

        // Special case: Normally, the selection extender should process the mouse release event *before* this tool, yet
        //               it is done *afterwards* so that it can successfully hide the mouse cursor point if the mouse
        //               cursor happens to be above one of the selection modifying handles.
        selectionExtender_->viewportMouseReleaseEvent(mouseEvent);
    }
    else if (mouseEvent->button() == Qt::RightButton)
    {
        collapseIsActive_ = false;
    }
}

/**
  * @param renderEngine [IRenderEngine *]
  * @param painter [QPainter *]
  */
void SelectMsaTool::viewportPaint(IRenderEngine *renderEngine, QPainter *painter)
{
    selectionExtender_->viewportPaint(renderEngine, painter);
}

/**
  */
void SelectMsaTool::viewportWindowDeactivate()
{
    slideIsActive_ = false;
    selectionIsActive_ = false;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void SelectMsaTool::onActionExtendSequence()
{
    ObservableMsa *msa = msaView_->msa();
    QPoint p = msaView_->mouseCursorPoint();

    if (p.x() <= msa->length() / 2.)
    {
        if (msa->at(p.y())->start() > 1)
            msaView_->undoStack()->push(new SetSubseqStartCommand(msa, p.y(), msa->at(p.y())->start() - 1));
    }
    else if (msa->at(p.y())->stop() < msa->at(p.y())->parentSeq_.length())
    {
        // Passing in the msaView as an argument ot SetSubseqStopCommand provides it with the ability to scroll
        // the horizontal scroll bar if it is at the maximum scroll value when the command is issued.
        msaView_->undoStack()->push(new SetSubseqStopCommand(msa, p.y(), msa->at(p.y())->stop() + 1, msaView_));
    }
}

/**
  */
void SelectMsaTool::onActionTrimSequence()
{
    ObservableMsa *msa = msaView_->msa();
    QPoint p = msaView_->mouseCursorPoint();
    if (p.x() <= msa->length() / 2.)
        msaView_->undoStack()->push(new SetSubseqStartCommand(msa, p.y(), msa->at(p.y())->start() + 1));
    else if (msa->at(p.y())->stop() > 1)
        msaView_->undoStack()->push(new SetSubseqStopCommand(msa, p.y(), msa->at(p.y())->stop() - 1));
}

/**
  */
void SelectMsaTool::onMsaSelectionScrollTimeout()
{
    ASSERT(selectionIsActive_);

    int dx = 0;
    int dy = 0;

    QPoint curMousePos = msaView_->mouseHotSpot();

    // Determine horizontal scroll
    bool mouseXOutsideViewport = curMousePos.x() < 0 || curMousePos.x() >= msaView_->viewport()->width();
    bool mouseYOutsideViewport = curMousePos.y() < 0 || curMousePos.y() >= msaView_->viewport()->height();
    if (mouseXOutsideViewport)
    {
        if (curMousePos.x() > viewAnchorPoint_.x())
            // Went off the right side
            dx = curMousePos.x() - msaView_->viewport()->width();
        else
            dx = curMousePos.x();

        msaView_->horizontalScrollBar()->setValue(msaView_->horizontalScrollBar()->value() + dx);
    }

    if (mouseYOutsideViewport)
    {
        if (curMousePos.y() > viewAnchorPoint_.y())
            // Went off the bottom
            dy = curMousePos.y() - msaView_->viewport()->height();
        else
            dy = curMousePos.y();

        msaView_->verticalScrollBar()->setValue(msaView_->verticalScrollBar()->value() + dy);
    }

    // Now that we have scrolled - update the selection
    updateStop(curMousePos);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Convenience method for finishing/terminating any "open" selection or slide operation. Currently, called from
  * viewportMouseReleaseEvent and deactivate.
  *
  * @see viewportMouseReleaseEvent(), deactivate()
  */
void SelectMsaTool::finishSelectionSlide()
{
    if (selectionIsActive_)
    {
        ASSERT(!slideIsActive_);

        selectionIsActive_ = false;
        msaSelectionScrollTimer_.stop();
        emit selectionFinished(msaView_->selection());
    }

    if (slideIsActive_)
    {
        ASSERT(!selectionIsActive_);
        slideIsActive_ = false;
        emit slideFinished(msaView_->selection());
    }
}

/**
  */
void SelectMsaTool::updateMouseCursor()
{
    QPoint mousePos = msaView_->viewport()->mapFromGlobal(QCursor::pos());

    if (selectionIsActive_)
    {
        msaView_->hideMouseCursorPoint();
        msaView_->viewport()->setCursor(Qt::ArrowCursor);
    }
    else    // Selection is not active
    {
        if (slideIsActive_ ||
            (msaView_->isMouseOverSelection() && !selectionExtender_->isPointOverControl(mousePos)))
        {
            msaView_->hideMouseCursorPoint();
            msaView_->viewport()->setCursor(Qt::SizeHorCursor);
        }
        else
        {
            msaView_->viewport()->setCursor(Qt::ArrowCursor);

            if (msaView_->selection().normalized().contains(msaView_->mouseCursorPoint()))
                msaView_->hideMouseCursorPoint();
            else
                msaView_->showMouseCursorPoint();
        }
    }

    if (!msaView_->viewport()->rect().contains(mousePos))
        msaView_->hideMouseCursorPoint();
}

/**
  * By default, stopPoint is null, which implies to autocalculate the stop position from the current mouse position
  * within the viewport.
  *
  * @param stopPoint [const QPoint &]
  */
void SelectMsaTool::updateStop(const QPoint &stopPoint)
{
    if (!selectionIsActive_)
        return;

    // Get the current selection in case we are constrained to one axis
    PosiRect currentSelection = msaView_->selection().normalized();

    QPoint start = msaStartAnchorPoint_;
    QPoint stop;
    if (stopPoint.isNull() == false)
        stop = pointRectMapper_.viewPointToMsaPoint(stopPoint);
    else
        stop = msaView_->mouseCursorPoint();

    if (selectionAxis_ == Ag::VerticalAxis)
    {
        stop.setX(currentSelection.right());

        // Note: column selection is not supported when the selection mode is constrained to the vertical axis
    }
    else
    {
        // Axis is either horizontal or both vertical and horizontal.
        if (selectionAxis_ == Ag::HorizontalAxis)
            stop.setY(currentSelection.bottom());

        // If Ctrl is pressed = column selection mode; move selection stop bottom to last sequence
        if (ctrlPressed_)
        {
            // Additionally, we set the selection start top to 1 to handle the case where the user starts a selection,
            // and then presses the Alt key
            start.ry() = 1;
            stop.ry() = msaView_->msa()->rowCount();
        }
    }

    msaView_->setSelection(PosiRect(start, stop));
    msaView_->viewport()->update();
}
