/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QTimer>

#include <QtGui/QHeaderView>
#include <QtGui/QLayout>
#include <QtGui/QScrollBar>

#include "AbstractMsaView.h"
#include "MsaTableView.h"
#include "../gui_misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
MsaTableView::MsaTableView(QWidget *parent)
    : SingleColumnTableView(parent),
      msaView_(nullptr)
{
}

/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QWidget *]
  */
MsaTableView::MsaTableView(AbstractMsaView *msaView, QWidget *parent)
    : SingleColumnTableView(parent),
      msaView_(nullptr)
{
    setMsaView(msaView);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param msaView [AbstractMsaView *]
  */
void MsaTableView::setMsaView(AbstractMsaView *msaView)
{
    if (msaView_ != nullptr)
    {
        disconnect(msaView_->verticalScrollBar(), SIGNAL(valueChanged(int)), verticalScrollBar(), SLOT(setValue(int)));
        disconnect(msaView_, SIGNAL(zoomChanged(double)), this, SLOT(resizeFont()));
        disconnect(msaView_, SIGNAL(fontChanged()), this, SLOT(resizeFont()));
        disconnect(verticalScrollBar(), SIGNAL(valueChanged(int)), msaView_->verticalScrollBar(), SLOT(setValue(int)));
    }
    msaView_ = msaView;
    if (msaView_ != nullptr)
    {
        connect(msaView_->verticalScrollBar(), SIGNAL(valueChanged(int)), verticalScrollBar(), SLOT(setValue(int)));
        connect(msaView_, SIGNAL(zoomChanged(double)), SLOT(resizeFont()));
        connect(msaView_, SIGNAL(fontChanged()), SLOT(resizeFont()));
        connect(verticalScrollBar(), SIGNAL(valueChanged(int)), msaView_->verticalScrollBar(), SLOT(setValue(int)));

        // Match the font to the view settings - but since resizeFont calls a virtual method of AbstractMsaView
        // call it in a timer in case this method is called from the AbstractMsaView's constructor
        QTimer::singleShot(0, this, SLOT(resizeFont()));
    }

    update();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected slots
/**
  */
void MsaTableView::resizeFont()
{
    ASSERT(msaView_ != nullptr);

    // When passing the number of pixels to use for the calculation, 2 less pixels are used in the calculation because
    // those characters with descenders (e.g. 'j', 'g', 'q') we're being partially obscured by the next line without
    // this change. Perhaps a better alternative would be to somehow make the text render without first painting its
    // whole background rectangle. In other words, paint the entire background of the widget first, and then repaint all
    // cells by only drawing their text. Not sure how this would work in conjunction with selection.
    QFont font = this->font();
    int nPixels = qMax(1., msaView_->charHeight() - 2);
    int pointSize = ::fitPointSizeFromPixels(font.family(), nPixels, logicalDpiY());
    font.setPointSize(pointSize);
    setFont(font);

    verticalHeader()->setDefaultSectionSize(msaView_->charHeight());
}
