/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef HANDMSATOOL_H
#define HANDMSATOOL_H

#include <QtCore/QPoint>

#include "AbstractMsaTool.h"
#include "../../core/global.h"

class QKeyEvent;
class QMouseEvent;

class AbstractMsaView;

class HandMsaTool : public AbstractMsaTool
{
public:
    HandMsaTool(AbstractMsaView *msaView, QObject *parent = nullptr);

    bool isActive() const;
    virtual int type() const;

public Q_SLOTS:
    void activate();

protected:
    virtual void viewportMousePressEvent(QMouseEvent *mouseEvent);
    virtual void viewportMouseMoveEvent(QMouseEvent *mouseEvent);
    virtual void viewportMouseReleaseEvent(QMouseEvent *mouseEvent);
    virtual void viewportWindowDeactivate();

private:
    QPoint lastPanScrollPosition_;
    QPoint panAnchorPoint_;
    bool isActive_;
};

#endif // HANDMSATOOL_H
