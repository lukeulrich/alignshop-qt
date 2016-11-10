/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IMSATOOL_H
#define IMSATOOL_H

#include <QtCore/QObject>

#include "../../core/global.h"

class QKeyEvent;
class QMouseEvent;
class QPainter;
class QWheelEvent;

class AbstractMsaView;
class IRenderEngine;

class IMsaTool : public QObject
{
    Q_OBJECT

public:
    virtual AbstractMsaView *msaView() const = 0;

    virtual bool isActive() const = 0;
    virtual int type() const = 0;

    // TODO:
//    virtual void paint(QPainter *painter);

public Q_SLOTS:
    virtual void activate() = 0;
    virtual void deactivate() = 0;

Q_SIGNALS:
    void activated();
    void deactivated();

protected:
    IMsaTool(QObject *parent = nullptr) : QObject(parent) {}

    // Viewport events for this tool
    virtual void viewportKeyPressEvent(QKeyEvent *keyEvent) = 0;
    virtual void viewportKeyReleaseEvent(QKeyEvent *keyEvent) = 0;
    virtual void viewportMouseDoubleClickEvent(QMouseEvent *mouseEvent) = 0;
    virtual void viewportMouseEnterEvent() = 0;
    virtual void viewportMouseLeaveEvent() = 0;
    virtual void viewportMouseMoveEvent(QMouseEvent *mouseEvent) = 0;
    virtual void viewportMousePressEvent(QMouseEvent *mouseEvent) = 0;
    virtual void viewportMouseReleaseEvent(QMouseEvent *mouseEvent) = 0;
    virtual void viewportPaint(IRenderEngine *renderEngine, QPainter *painter) = 0;                // This is called after all other painting has completed
    virtual void viewportWheelEvent(QWheelEvent *wheelEvent) = 0;
    virtual void viewportWindowDeactivate() = 0;

    // Enable the MsaView to call this classes private methods directly
    friend class AbstractMsaView;
};

#endif // IMSATOOL_H
