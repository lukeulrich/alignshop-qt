/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>

#include "MsaVertSelectionBar.h"
#include "AbstractMsaView.h"

#include "../MsaTools/IMsaTool.h"
#include "../MsaTools/MsaToolTypes.h"

#include "../../core/ObservableMsa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
MsaVertSelectionBar::MsaVertSelectionBar(QWidget *parent)
    : AbstractMsaSideWidget(parent),
      showMouseBar_(false)
{
}

/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QWidget *]
  */
MsaVertSelectionBar::MsaVertSelectionBar(AbstractMsaView *msaView, QWidget *parent)
    : AbstractMsaSideWidget(parent),
      showMouseBar_(false)
{
    // Note we do not call the two-argument parent constructor. That is because setMsaView is not a virtual method
    // (nor should it be if called from a constructor) and this class has special signals to watch the msa view.
    setMsaView(msaView);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param msaView [AbstractMsaView *]
  */
void MsaVertSelectionBar::setMsaView(AbstractMsaView *msaView)
{
    if (msaView_ != nullptr)
    {
        disconnect(msaView_, SIGNAL(selectionChanged(PosiRect,PosiRect)), this, SLOT(onMsaSelectionChanged(PosiRect)));
        disconnect(msaView_, SIGNAL(mouseCursorMoved(QPoint,QPoint)), this, SLOT(update()));
        disconnect(msaView_, SIGNAL(mouseCursorVisibleChanged(bool)), this, SLOT(onMsaMouseCursorVisibleChanged(bool)));
        disconnect(msaView_, SIGNAL(msaToolChanged()), this, SLOT(update()));
    }

    AbstractMsaSideWidget::setMsaView(msaView);

    ASSERT(msaView == msaView_);
    if (msaView != nullptr)
    {
        connect(msaView_, SIGNAL(selectionChanged(PosiRect,PosiRect)), SLOT(onMsaSelectionChanged(PosiRect)));
        connect(msaView_, SIGNAL(mouseCursorMoved(QPoint,QPoint)), SLOT(update()));
        connect(msaView_, SIGNAL(mouseCursorVisibleChanged(bool)), SLOT(onMsaMouseCursorVisibleChanged(bool)));
        connect(msaView_, SIGNAL(msaToolChanged()), SLOT(update()));

        // Simulate selection changed event
        verticalSelection_ = ClosedIntRange();
        onMsaSelectionChanged(msaView->selection());
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param painter [QPainter *]
  * @param rowHeight [const int]
  * @param baseline [const double]
  * @param startY [const int]
  * @param startMsaRow [const int]
  */
void MsaVertSelectionBar::paintEvent(QPainter *painter, const int rowHeight, const double baseline, const int startY, const int startMsaRow)
{
    Q_UNUSED(baseline)

    if (msaView_ == nullptr || msaView_->msa() == nullptr)
        return;

    if (verticalSelection_.isEmpty())
        return;

    int lastMsaRow = startMsaRow + ((height() - startY) / rowHeight);
    if (lastMsaRow > msaView_->msa()->rowCount())
        lastMsaRow = msaView_->msa()->rowCount();

    bool selectionIsInRange = startMsaRow >= verticalSelection_.begin_ ||
                              startMsaRow <= verticalSelection_.end_ ||
                              lastMsaRow >= verticalSelection_.begin_ ||
                              lastMsaRow <= verticalSelection_.end_;
    if (!selectionIsInRange)
        return;

    // Selection is in range, determine the vertical start and stop render positions
    int y1 = startY + (verticalSelection_.begin_ - startMsaRow) * rowHeight;
    int renderSelectionHeight = verticalSelection_.length() * rowHeight;

    painter->setBrush(palette().color(QPalette::Highlight));
    painter->setPen(Qt::NoPen);
    painter->drawRect(2, y1, width() - 4, renderSelectionHeight);

    // -----------------------------------------------
    // Now render the rectangle denoting the mouse bar, but only if the SelectMsaTool is current
    if (showMouseBar_)
    {
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawRect(2, startY + (msaView_->mouseCursorPoint().y() - startMsaRow) * rowHeight, width() - 4, rowHeight);
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * Always show the mouse bar if the mouse cursor is visible including when the mouse cursor is over the selection.
  * ISSUE: Check if the currentMsaTool is the SelectMsaTool?
  *
  * @param visible [bool]
  */
void MsaVertSelectionBar::onMsaMouseCursorVisibleChanged(bool visible)
{
    showMouseBar_ = visible || msaView_->currentMsaTool()->type() == Ag::kSelectMsaTool;
    update();
}

/**
  * @param currentSelection [const PosiRect &]
  */
void MsaVertSelectionBar::onMsaSelectionChanged(const PosiRect &currentSelection)
{
    if (currentSelection.normalized().verticalRange() == verticalSelection_)
        return;

    // OPTIMIZATION: Could be more strategic and only update if the old vertical range was visible and/or the selection
    // range is visible. But for now, always issue an update. It's simply one rectangle - so it will never be costly.
    verticalSelection_ = currentSelection.normalized().verticalRange();
    update();
}
