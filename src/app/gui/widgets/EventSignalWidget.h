/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef EVENTSIGNALWIDGET_H
#define EVENTSIGNALWIDGET_H

#include <QtGui/QResizeEvent>
#include <QtGui/QWidget>
#include "../../core/global.h"

/**
  * EventSignalWidget is a utility widget that provides signals corresponding to basic events that are not normally
  * exposed via the signal/slot mechanism.
  */
class EventSignalWidget : public QWidget
{
    Q_OBJECT

public:
    EventSignalWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }

Q_SIGNALS:
    void resized(const QSize &size, const QSize &oldSize);

protected:
    virtual void resizeEvent(QResizeEvent *resizeEvent)
    {
        emit resized(resizeEvent->size(), resizeEvent->oldSize());
    }
};

#endif // EVENTSIGNALWIDGET_H
