/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTMSATOOL_H
#define ABSTRACTMSATOOL_H

#include "IMsaTool.h"
#include "MsaToolTypes.h"
#include "../../core/global.h"

class QKeyEvent;
class QMouseEvent;
class QPainter;
class QWheelEvent;

class AbstractMsaView;

class AbstractMsaTool : public IMsaTool
{
    Q_OBJECT

public:
    AbstractMsaView *msaView() const;

    virtual bool isActive() const                                           { return false; }
    virtual int type() const                                                { return Ag::kUnknownMsaTool; }

public Q_SLOTS:
    virtual void activate();
    virtual void deactivate();

protected:
    AbstractMsaTool(AbstractMsaView *msaView, QObject *parent = nullptr);

    virtual void viewportKeyPressEvent(QKeyEvent * /* event */)             {}
    virtual void viewportKeyReleaseEvent(QKeyEvent * /* event */)           {}
    virtual void viewportMouseDoubleClickEvent(QMouseEvent * /* event */)   {}
    virtual void viewportMouseEnterEvent()                                  {}
    virtual void viewportMouseLeaveEvent()                                  {}
    virtual void viewportMouseMoveEvent(QMouseEvent * /* event */)          {}
    virtual void viewportMousePressEvent(QMouseEvent * /* event */)         {}
    virtual void viewportMouseReleaseEvent(QMouseEvent * /* event */)       {}
    virtual void viewportPaint(IRenderEngine * /* renderEngine */, QPainter * /* painter */)          {}
    virtual void viewportWheelEvent(QWheelEvent * /* event */)              {}
    virtual void viewportWindowDeactivate()                                 {}

    AbstractMsaView *msaView_;
};

#endif // ABSTRACTMSATOOL_H
