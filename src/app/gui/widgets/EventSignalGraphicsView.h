/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef EVENTSIGNALGRAPHICSVIEW_H
#define EVENTSIGNALGRAPHICSVIEW_H

#include <QtGui/QGraphicsView>
#include <QtGui/QResizeEvent>
#include "../../core/global.h"

class EventSignalGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    EventSignalGraphicsView(QWidget *parent = nullptr)
        : QGraphicsView(parent)
    {
    }


Q_SIGNALS:
    void viewportResized(const QSize &size, const QSize &oldSize);
    void wheelDelta(int delta, int degrees, int steps);


protected:
    virtual void resizeEvent(QResizeEvent *resizeEvent)
    {
        emit viewportResized(resizeEvent->size(), resizeEvent->oldSize());
        QGraphicsView::resizeEvent(resizeEvent);
    }

    virtual void wheelEvent(QWheelEvent *event)
    {
        int degrees = event->delta() / 8;
        int steps = degrees / 15;

        emit wheelDelta(event->delta(), degrees, steps);
        QGraphicsView::wheelEvent(event);
    }

};

#endif // EVENTSIGNALGRAPHICSVIEW_H
