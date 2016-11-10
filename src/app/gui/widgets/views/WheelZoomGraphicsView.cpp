/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QWheelEvent>

#include "WheelZoomGraphicsView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
WheelZoomGraphicsView::WheelZoomGraphicsView(QWidget *parent)
    : EventSignalGraphicsView(parent),
      zoomFactor_(.05),
      modifier_(Qt::ControlModifier)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Qt::KeyboardModifier
  */
Qt::KeyboardModifier WheelZoomGraphicsView::modifier() const
{
    return modifier_;
}

/**
  * @returns double
  */
double WheelZoomGraphicsView::zoomFactor() const
{
    return zoomFactor_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param newModifier [const Qt::KeyboardModifier]
  */
void WheelZoomGraphicsView::setModifier(const Qt::KeyboardModifier newModifier)
{
    modifier_ = newModifier;
}

/**
  * @param newZoomFactor [const double]
  */
void WheelZoomGraphicsView::setZoomFactor(const double newZoomFactor)
{
    zoomFactor_ = newZoomFactor;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param event [QWheelEvent *]
  */
void WheelZoomGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & modifier_)
    {
        if (event->delta() > 0)
            scale(1. + zoomFactor_, 1. + zoomFactor_);
        else
            scale(1. / (1. + zoomFactor_), 1. / (1. + zoomFactor_));

        event->accept();
    }
    else
    {
        EventSignalGraphicsView::wheelEvent(event);
    }
}
