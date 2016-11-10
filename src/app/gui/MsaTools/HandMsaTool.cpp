/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QScrollBar>

#include "HandMsaTool.h"
#include "MsaToolTypes.h"
#include "../widgets/AbstractMsaView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QObject *]
  */
HandMsaTool::HandMsaTool(AbstractMsaView *msaView, QObject *parent)
    : AbstractMsaTool(msaView, parent), isActive_(false)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool HandMsaTool::isActive() const
{
    return isActive_;
}

/**
  * @returns int
  */
int HandMsaTool::type() const
{
    return Ag::kHandMsaTool;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void HandMsaTool::activate()
{
    msaView_->viewport()->setCursor(Qt::OpenHandCursor);

    AbstractMsaTool::activate();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param mouseEvent [QMouseEvent *]
  */
void HandMsaTool::viewportMousePressEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        msaView_->viewport()->setCursor(Qt::ClosedHandCursor);
        panAnchorPoint_ = mouseEvent->pos();
        lastPanScrollPosition_ = msaView_->scrollPosition();
        isActive_ = true;
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void HandMsaTool::viewportMouseMoveEvent(QMouseEvent *mouseEvent)
{
    if (isActive_)
    {
        int dx = mouseEvent->x() - panAnchorPoint_.x();
        int dy = mouseEvent->y() - panAnchorPoint_.y();

        msaView_->horizontalScrollBar()->setValue(lastPanScrollPosition_.x() - dx);
        msaView_->verticalScrollBar()->setValue(lastPanScrollPosition_.y() - dy);

        panAnchorPoint_ = mouseEvent->pos();
        lastPanScrollPosition_ = msaView_->scrollPosition();
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void HandMsaTool::viewportMouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        msaView_->viewport()->setCursor(Qt::OpenHandCursor);
        isActive_ = false;
    }
}

/**
  */
void HandMsaTool::viewportWindowDeactivate()
{
    isActive_ = false;
    msaView_->viewport()->setCursor(Qt::OpenHandCursor);
}
