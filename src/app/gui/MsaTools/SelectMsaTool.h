/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SELECTMSATOOL_H
#define SELECTMSATOOL_H

#include <QtCore/QPoint>
#include <QtCore/QSet>
#include <QtCore/QTimer>

#include "AbstractMsaTool.h"
#include "../util/PointRectMapper.h"
#include "../../core/global.h"

class QAction;
class QMouseEvent;

class AbstractMsaView;
class HandMsaTool;

namespace Ag
{
    enum Axis
    {
        HorizontalAxis = 0,
        VerticalAxis,
        HorizontalVerticalAxis
    };
}

// Special private helper class
class SelectionExtenderPrivate;

/**
  *
  *
  * TODO:
  * o Migrate to a state machine
  */
class SelectMsaTool : public AbstractMsaTool
{
    Q_OBJECT

public:
    SelectMsaTool(AbstractMsaView *msaView, QObject *parent = nullptr);

    QAction *extendSequenceAction() const;
    bool isActive() const;
    void setHandMsaTool(HandMsaTool *handMsaTool);
    void setSelectionAxis(const Ag::Axis axis);                     //!< Contrains the selection axis to axis
    QAction *trimSequenceAction() const;
    virtual int type() const;

public Q_SLOTS:
    void activate();
    void deactivate();

Q_SIGNALS:
    void selectionCleared();                                        //!< Emitted when the user has cleared the selection
    void selectionStarted();                                        //!< Emitted when the user has begun a selection with the mouse
    void selectionFinished(const PosiRect &currentSelection);       //!< Emitted just after the user has finished making a selection with the mouse
    void slideStarted(const PosiRect &msaRect);                     //!< Emitted when the region in msaRect has been initiated for a slide operation
    void slideFinished(const PosiRect &msaRect);                    //!< Emitted after the region in msaRect has finished sliding

protected:
    virtual void viewportKeyPressEvent(QKeyEvent *keyEvent);
    virtual void viewportKeyReleaseEvent(QKeyEvent *keyEvent);
    virtual void viewportMouseLeaveEvent();
    virtual void viewportMousePressEvent(QMouseEvent *mouseEvent);
    virtual void viewportMouseMoveEvent(QMouseEvent *mouseEvent);
    virtual void viewportMouseReleaseEvent(QMouseEvent *mouseEvent);
    virtual void viewportPaint(IRenderEngine *renderEngine, QPainter *painter);
    virtual void viewportWindowDeactivate();

private Q_SLOTS:
    void onActionExtendSequence();
    void onActionTrimSequence();
    void onMsaSelectionScrollTimeout();

private:
    void finishSelectionSlide();                                    //!< Close down any pending selection or slide operation
    void updateMouseCursor();
    void updateStop(const QPoint &stopPoint = QPoint());

    bool selectionIsActive_;
    bool slideIsActive_;
    bool collapseIsActive_;
    QPoint msaStartAnchorPoint_;    // Anchor point in msa space for the selection start
    QPoint viewAnchorPoint_;        // Anchor point in view space
    Ag::Axis selectionAxis_;        // Axis to constrain selection
    QPoint slideMsaAnchorPoint_;
    PointRectMapper pointRectMapper_;
    QTimer msaSelectionScrollTimer_;

    QPoint viewCollapseAnchorPoint_;

    bool ctrlPressed_;

    // Support for temporary switch to the hand tool
    HandMsaTool *handMsaTool_;

    QSet<int> keysPressed_;

    // Support for the trim / extend a single sequence action
    QAction *extendSequenceAction_;
    QAction *trimSequenceAction_;

    // Special class for modifying a given selection rectangle via handles
    SelectionExtenderPrivate *selectionExtender_;
    friend class SelectionExtenderPrivate;
};

#endif // SELECTMSATOOL_H
