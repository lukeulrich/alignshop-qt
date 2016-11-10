/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ZOOMMSATOOL_H
#define ZOOMMSATOOL_H

#include "AbstractMsaTool.h"

class ZoomMsaTool : public AbstractMsaTool
{
    Q_OBJECT

public:
    ZoomMsaTool(AbstractMsaView *msaView, QObject *parent = nullptr);

    bool isActive() const;
    int type() const;

public Q_SLOTS:
    void activate();

protected:
    virtual void viewportKeyPressEvent(QKeyEvent *keyEvent);
    virtual void viewportKeyReleaseEvent(QKeyEvent *keyEvent);
    virtual void viewportMousePressEvent(QMouseEvent *mouseEvent);
    virtual void viewportMouseReleaseEvent(QMouseEvent *mouseEvent);

private:
    bool isActive_;
    bool shiftPressed_;
};

#endif // ZOOMMSATOOL_H
