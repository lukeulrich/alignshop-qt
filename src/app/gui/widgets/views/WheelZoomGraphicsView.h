/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef WHEELZOOMGRAPHICSVIEW_H
#define WHEELZOOMGRAPHICSVIEW_H

#include "../EventSignalGraphicsView.h"
#include "../../../core/global.h"

#include <QtDebug>

/**
  * WheelZoomGraphicsView extends the QGraphicsView and provides for scaling the entire view by zoomFactor when a user
  * defined keyboard modifier is pressed while moving the mouse wheel.
  *
  * By default, the keyboard modifier is Control.
  */
class WheelZoomGraphicsView : public EventSignalGraphicsView
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit WheelZoomGraphicsView(QWidget *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    Qt::KeyboardModifier modifier() const;
    double zoomFactor() const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setModifier(const Qt::KeyboardModifier newModifier);
    void setZoomFactor(const double newZoomFactor);


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void zoomFactorChanged(const double zoomFactor);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
//    virtual void keyPressEvent(QKeyEvent *event);
//    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void wheelEvent(QWheelEvent *event);


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    double zoomFactor_;
    Qt::KeyboardModifier modifier_;
};

#endif // WHEELZOOMGRAPHICSVIEW_H
