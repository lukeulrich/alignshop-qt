/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "MsaToolTypes.h"
#include "ZoomMsaTool.h"
#include "../widgets/AbstractMsaView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QObject *]
  */
ZoomMsaTool::ZoomMsaTool(AbstractMsaView *msaView, QObject *parent)
    : AbstractMsaTool(msaView, parent),
      isActive_(false),
      shiftPressed_(false)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool ZoomMsaTool::isActive() const
{
    return isActive_;
}

/**
  * @returns int
  */
int ZoomMsaTool::type() const
{
    return Ag::kZoomMsaTool;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void ZoomMsaTool::activate()
{
    msaView_->viewport()->setCursor(QCursor(QPixmap(":/aliases/images/cursors/zoom-in")));

    AbstractMsaTool::activate();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param keyEvent [QKeyEvent *]
  */
void ZoomMsaTool::viewportKeyPressEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_Shift && !keyEvent->isAutoRepeat())
    {
        msaView_->viewport()->setCursor(QCursor(QPixmap(":/aliases/images/cursors/zoom-out")));
        shiftPressed_ = true;
    }
}

/**
  * @param keyEvent [QKeyEvent *]
  */
void ZoomMsaTool::viewportKeyReleaseEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_Shift && !keyEvent->isAutoRepeat())
    {
        msaView_->viewport()->setCursor(QCursor(QPixmap(":/aliases/images/cursors/zoom-in")));
        shiftPressed_ = false;
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void ZoomMsaTool::viewportMousePressEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        isActive_ = true;
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void ZoomMsaTool::viewportMouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        isActive_ = true;
        msaView_->setZoomBy((shiftPressed_) ? -1 : 1, mouseEvent->pos());
    }
    isActive_ = false;
}
