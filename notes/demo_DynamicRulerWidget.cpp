/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <cmath>        // For pow

#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>
#include <QtGui/QStyleOption>

#include "MsaRulerWidget.h"
#include "AbstractMsaView.h"

#include "../../core/ObservableMsa.h"
#include "../../core/misc.h"

static const int kVerticalSize = 25;    // Pixels, excludes padding
static const int kMinimumUnitsPerLabel = 5;     // No label may be less than 5 units (characters) apart

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
MsaRulerWidget::MsaRulerWidget(QWidget *parent)
    : QWidget(parent),
      msaView_(nullptr),
      verticalPadding_(0)
{
    setFixedHeight(kVerticalSize);
}

/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QWidget *]
  */
MsaRulerWidget::MsaRulerWidget(AbstractMsaView *msaView, QWidget *parent)
    : QWidget(parent),
      msaView_(nullptr),
      verticalPadding_(0)
{
    setFixedHeight(kVerticalSize);
    setMsaView(msaView);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param msaView [AbstractMsaView *]
  */
void MsaRulerWidget::setMsaView(AbstractMsaView *msaView)
{
    if (msaView_ != nullptr)
        disconnect(msaView_->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));

    msaView_ = msaView;

    if (msaView_ != nullptr)
        connect(msaView_->horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(update()));

    update();
}

/**
  * @param verticalPadding [const int]
  */
void MsaRulerWidget::setVerticalPadding(const int verticalPadding)
{
    ASSERT(verticalPadding >= 0);

    if (verticalPadding_ != verticalPadding)
    {
        verticalPadding_ = verticalPadding;
        update();
    }
}

/**
  * @returns int
  */
int MsaRulerWidget::verticalPadding() const
{
    return verticalPadding_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param paintEvent [QPaintEvent *]
  */
void MsaRulerWidget::paintEvent(QPaintEvent * /* paintEvent */)
{
    if (msaView_ == nullptr || msaView_->msa() == nullptr)
        return;

    // Zoom is irrelevant here because msaView_->charWidth() returns the scaled width in pixels

    int dpiX = logicalDpiX();
    int columnWidth = msaView_->charWidth();
    int horizScrollPos = msaView_->horizontalScrollBar()->value();
    int offset = -horizScrollPos % columnWidth;
    int startColumn = (horizScrollPos / columnWidth) + 1;  // Convert to 1-based msa indices
    int unitsPerLabel = this->unitsPerLabel(columnWidth, dpiX);
    ASSERT(unitsPerLabel > 0);

    QStyleOption option;
    option.init(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
    QFont font("Sans", 7);
    painter.setFont(font);
    QFontMetrics fontMetrics(font);

    // Special case: draw 1 at the very left hand side of align ment
    int x = offset;
    int column = 0;
    if (startColumn == 1)
    {
        painter.drawText(x, fontMetrics.ascent(), "1");
        x += (unitsPerLabel - 1) * columnWidth;
        column = unitsPerLabel;
    }
    else
    {
        column = (static_cast<int>(startColumn / unitsPerLabel)) * unitsPerLabel;
        x += (column - startColumn) * columnWidth;
    }

    for (int w = width(), nCols = msaView_->msa()->length(); x < w && column < nCols; column += unitsPerLabel)
    {
        painter.drawText(x, fontMetrics.ascent(), QString::number(column));
        x += unitsPerLabel * columnWidth;
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param columnWidth [const int]
  * @param dpiX [const int]
  * @returns int
  */
int MsaRulerWidget::unitsPerLabel(const int columnWidth, const int dpiX) const
{
    int minLabelSeparationPx = dpiX * .7;
    int maxLabelSeparationPx = dpiX * 1.3;

    // We begin this loop with the smallest acceptable units per label (which is 5 because the for loop starts with
    // i == 2). Consequently, there is no need to attempt to reduce the nUnitsPerLabel - it simply isn't possible.
    for (int i=2, mod = i % 3; i; ++i, mod = i % 3)
    {
        // Only permit labels at factors of 1, 2, or 5
        int base = 1;
        if (mod == 1)
            base = 2;
        else if (mod == 2)
            base = 5;

        int nUnitsPerLabel = base * pow(10, i / 3);
        int nPixelsPerLabel = nUnitsPerLabel * columnWidth;

        if (nPixelsPerLabel > maxLabelSeparationPx)
            return nUnitsPerLabel;

        // On the other hand, check that the current nUnitsPerLabel fits within the requested range.
        if (nPixelsPerLabel >= minLabelSeparationPx)
            return nUnitsPerLabel;

        // Did not fit, loop again to increase the nUnitsPerLabel (this corresponds to the case where the user has
        // zoomed out).
    }

    ASSERT(0);
    return -1;
}
