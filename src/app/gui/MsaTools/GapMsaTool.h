/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GAPMSATOOL_H
#define GAPMSATOOL_H

#include <QtCore/QPoint>

#include "AbstractMsaTool.h"
#include "../../core/global.h"
#include "../util/PointRectMapper.h"

class QKeyEvent;
class QMouseEvent;
class QPainter;

class AbstractMsaView;
class IRenderEngine;

// TODO: Remove gaps within selection range

/**
  * The GapMsaTool is quite complicated in light of the fact that we support adding gaps in either direction of the
  * mouse movement. Inserting gaps to the right of the origin (where the user clicks and then begins the drag), is
  * very straightforward - simply add the gaps and increase the scroll bar as necessary. It is also relatively easy to
  * add maintain expectations when the Msa is larger than the current view and thus a horizontal scrollbar is present.
  * Simply insert the gaps and then adjust the scrollbar so that it appears like gaps are being inserted to the left of
  * the origin.
  *
  * The most complicated case occurs when the Msa fits completely within the viewport and there are no scrollbars. It is
  * not immediately clear how to handle the situation where the user drags the mouse to the left. The current solution
  * is as follows:
  * 1) Detect if the horizontal scroll bar is present when the user clicks the left mouse button
  * 2) If yes, continue as before, if no:
  *    a) Prevent the scrollbar from appearing
  *    b) Do not update the scrollbar during mouse move events
  *    c) Calculate the amount to translate the view rendering when gaps are being added or removed from the left of the
  *       origin such that it appears as though the gaps are being inserted to the left
  *    d) On release of the left mouse button, turn on the scrollbar, set the translation amount to zero and re-render
  *
  * To make the above happen it is necessary to provide the ability to tweak the actual rendering origin of the
  * AbstractMsaView.
  */
class GapMsaTool : public AbstractMsaTool
{
    Q_OBJECT

public:
    GapMsaTool(AbstractMsaView *msaView, QObject *parent = nullptr);

    bool isActive() const;
    virtual int type() const;

public Q_SLOTS:
    void activate();

Q_SIGNALS:
    // normal -> range is normally positive; mouse started at point that was less than the end; vice versa
    void gapColumnsInsertStarted(const ClosedIntRange &columns, bool normal);
    void gapColumnsIntermediate(const ClosedIntRange &columns, bool normal);
    void gapColumnsInsertFinished(const ClosedIntRange &columns, bool normal);
    void gapInsertionColumnChanged(int column);

protected:
    virtual void viewportKeyPressEvent(QKeyEvent *keyEvent);
    virtual void viewportKeyReleaseEvent(QKeyEvent *keyEvent);
    virtual void viewportMouseLeaveEvent();
    virtual void viewportMousePressEvent(QMouseEvent *mouseEvent);
    virtual void viewportMouseMoveEvent(QMouseEvent *mouseEvent);
    virtual void viewportMouseReleaseEvent(QMouseEvent *mouseEvent);
    virtual void viewportPaint(IRenderEngine *renderEngine, QPainter *painter);
    virtual void viewportWindowDeactivate();

private:
    int gapInsertionColumn() const;                //!< Returns the position in msa space where a gap will be inserted based on the current mouse position
    qreal gapPlotX(int msaColumn) const;        // Returns the x position in viewport space for msaColumn to display the gap line

    bool isActive_;
    bool controlPressed_;
    int gapInsertionAnchorX_;

    PointRectMapper pointRectMapper_;

    int gapStartViewX_;
    int gapLastViewX_;
    int gapsAdded_;

    bool horizScrollBarVisible_;
};

#endif // GAPMSATOOL_H
