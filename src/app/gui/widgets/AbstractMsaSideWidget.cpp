/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QTimer>

#include <QtGui/QPainter>
#include <QtGui/QScrollBar>
#include <QtGui/QStyleOption>

#include "AbstractMsaSideWidget.h"
#include "AbstractMsaView.h"
#include "MsaRulerWidget.h"
#include "../gui_misc.h"


#include "../../core/ObservableMsa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
AbstractMsaSideWidget::AbstractMsaSideWidget(QWidget *parent)
    : QWidget(parent),
      msaView_(nullptr),
      alignment_(Qt::AlignLeft),
      horizontalPadding_(0)
{
}

/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QWidget *]
  */
AbstractMsaSideWidget::AbstractMsaSideWidget(AbstractMsaView *msaView, QWidget *parent)
    : QWidget(parent),
      msaView_(nullptr),
      alignment_(Qt::AlignLeft),
      horizontalPadding_(0)
{
    setMsaView(msaView);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Qt::Alignment
  */
Qt::Alignment AbstractMsaSideWidget::alignment() const
{
    return alignment_;
}

/**
  * @returns int
  */
int AbstractMsaSideWidget::horizontalPadding() const
{
    return horizontalPadding_;
}

/**
  * @param alignment [const Qt::Alignment]
  */
void AbstractMsaSideWidget::setAlignment(const Qt::Alignment alignment)
{
    if (alignment_ != alignment)
    {
        alignment_ = alignment;
        update();
    }
}

/**
  * @param horizontalPadding [int]
  */
void AbstractMsaSideWidget::setHorizontalPadding(const int horizontalPadding)
{
    ASSERT(horizontalPadding >= 0);

    if (horizontalPadding_ != horizontalPadding)
    {
        horizontalPadding_ = horizontalPadding;
        update();
    }
}

/**
  * @param msaView [AbstractMsaView *]
  */
void AbstractMsaSideWidget::setMsaView(AbstractMsaView *msaView)
{
    if (msaView_ != nullptr)
    {
        disconnect(msaView_->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));
        disconnect(msaView_, SIGNAL(zoomChanged(double)), this, SLOT(resizeFont()));
        disconnect(msaView_, SIGNAL(fontChanged()), this, SLOT(resizeFont()));
    }

    msaView_ = msaView;

    if (msaView_ != nullptr)
    {
        connect(msaView_->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(update()));
        connect(msaView_, SIGNAL(zoomChanged(double)), SLOT(resizeFont()));
        connect(msaView_, SIGNAL(fontChanged()), SLOT(resizeFont()));

        // Match the font to the view settings - but since resizeFont calls a virtual method of AbstractMsaView
        // call it in a timer in case this method is called from the AbstractMsaView's constructor
        QTimer::singleShot(0, this, SLOT(resizeFont()));
    }

    update();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param paintEvent [QPaintEvent *]
  */
void AbstractMsaSideWidget::paintEvent(QPaintEvent * /* paintEvent */)
{
    if (msaView_ == nullptr || msaView_->msa() == nullptr)
        return;

    int topMargin = 0;
    if (msaView_->ruler()->isVisible())
        topMargin = msaView_->ruler()->height();

    double rowHeight = msaView_->charHeight();
    double baseline = msaView_->baseline();
    int vertScrollPos = msaView_->verticalScrollBar()->value();

    int y = -vertScrollPos % static_cast<int>(rowHeight) + topMargin;
    int row = (vertScrollPos / rowHeight) + 1;  // Convert to 1-based msa indices

    QStyleOption option;
    option.init(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

    // Clip by default if the horizontal scroll bar is visible
    if (msaView_->horizontalScrollBar()->isVisible())
        painter.setClipRect(0, topMargin, width(), height() - msaView_->horizontalScrollBar()->height() - topMargin);
    else
        painter.setClipRect(0, topMargin, width(), height() - topMargin);

    this->paintEvent(&painter, rowHeight, baseline, y, row);
}

/**
  * @param painter [QPainter *]
  * @param rowHeight [const int]
  * @param baseline [const double]
  * @param startY [const int]
  * @param startMsaRow [const int]
  */
void AbstractMsaSideWidget::paintEvent(QPainter *painter, const int rowHeight, const double baseline, const int startY, const int startMsaRow)
{
    Q_UNUSED(painter);
    Q_UNUSED(rowHeight);
    Q_UNUSED(baseline);
    Q_UNUSED(startY);
    Q_UNUSED(startMsaRow);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected slots
/**
  */
void AbstractMsaSideWidget::resizeFont()
{
    ASSERT(msaView_ != nullptr);

    QFont font = this->font();
    int pointSize = ::fitPointSizeFromPixels(font.family(),
                                             msaView_->charHeight(),
                                             logicalDpiY());
    font.setPointSize(pointSize);
    setFont(font);
}
