/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QtEvents>
#include <QtGui/QFocusEvent>
#include <QtGui/QWidget>

#include "WidgetFocusObserver.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param target [QWidget *]
  * @param parent [QObject *]
  */
WidgetFocusObserver::WidgetFocusObserver(QWidget *target, QObject *parent)
    : QObject(parent)
{
	if (target != nullptr)
		target->installEventFilter(this);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param object [QObject *]
  * @param event [QEvent *]
  * @returns bool
  */
bool WidgetFocusObserver::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
        emit focusGained(static_cast<QFocusEvent *>(event)->reason());
    else if (event->type() == QEvent::FocusOut)
        emit focusLost(static_cast<QFocusEvent *>(event)->reason());

    return QObject::eventFilter(object, event);
}
