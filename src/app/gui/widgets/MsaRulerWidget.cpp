/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QTimer>

#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QPolygon>
#include <QtGui/QScrollBar>
#include <QtGui/QStyleOption>

#include "MsaRulerWidget.h"
#include "AbstractMsaView.h"

#include "../../core/ObservableMsa.h"
#include "../gui_misc.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
MsaRulerWidget::MsaRulerWidget(QWidget *parent)
    : QWidget(parent),
      msaView_(nullptr),
      bottomVerticalPadding_(0),
      unitsPerLabel_(10)
{
    setMouseTracking(true);
}

/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QWidget *]
  */
MsaRulerWidget::MsaRulerWidget(AbstractMsaView *msaView, QWidget *parent)
    : QWidget(parent),
      msaView_(nullptr),
      bottomVerticalPadding_(0),
      unitsPerLabel_(10)
{
    setMsaView(msaView);
    setMouseTracking(true);
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
    {
        disconnect(msaView_->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));
        disconnect(msaView_, SIGNAL(zoomChanged(double)), this, SLOT(resizeFont()));
        disconnect(msaView_, SIGNAL(zoomChanged(double)), this, SLOT(updateHeight()));
        disconnect(msaView_, SIGNAL(fontChanged()), this, SLOT(resizeFont()));
        disconnect(msaView_, SIGNAL(viewportMouseMoved(QPoint)), this, SLOT(onViewportMouseMoved(QPoint)));
        disconnect(msaView_, SIGNAL(mouseCursorMoved(QPoint,QPoint)), this, SLOT(update()));
    }

    msaView_ = msaView;

    if (msaView_ != nullptr)
    {
        connect(msaView_->horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(update()));
        connect(msaView_, SIGNAL(zoomChanged(double)), SLOT(resizeFont()));
        connect(msaView_, SIGNAL(zoomChanged(double)), SLOT(updateHeight()));
        connect(msaView_, SIGNAL(fontChanged()), SLOT(resizeFont()));
        connect(msaView_, SIGNAL(viewportMouseMoved(QPoint)), SLOT(onViewportMouseMoved(QPoint)));
        connect(msaView_, SIGNAL(mouseCursorMoved(QPoint,QPoint)), SLOT(update()));

        // Match the font to the view settings - but since resizeFont calls a virtual method of AbstractMsaView
        // call it in a timer in case this method is called from the AbstractMsaView's constructor
        QTimer::singleShot(0, this, SLOT(resizeFont()));
        QTimer::singleShot(0, this, SLOT(updateHeight()));
    }

    update();
}

/**
  * @param unitsPerLabel [int]
  */
void MsaRulerWidget::setUnitsPerLabel(const int unitsPerlabel)
{
    ASSERT(unitsPerlabel > 0);
    if (unitsPerLabel_ == unitsPerlabel)
        return;

    unitsPerLabel_ = unitsPerlabel;
    update();
}

/**
  * @param verticalPadding [const int]
  */
void MsaRulerWidget::setBottomVerticalPadding(const int verticalPadding)
{
    ASSERT(verticalPadding >= 0);

    if (bottomVerticalPadding_ != verticalPadding)
    {
        bottomVerticalPadding_ = verticalPadding;
        update();
    }
}

/**
  * @returns int
  */
int MsaRulerWidget::unitsPerLabel() const
{
    return unitsPerLabel_;
}

/**
  * @returns int
  */
int MsaRulerWidget::bottomVerticalPadding() const
{
    return bottomVerticalPadding_;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void MsaRulerWidget::updateHeight()
{
    setFixedHeight(QFontMetrics(font()).height() + bottomVerticalPadding_ + tickHeight());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param mouseEvent [QMouseEvent *]
  */
void MsaRulerWidget::mouseMoveEvent(QMouseEvent * /* mouseEvent */)
{
    update();
}

/**
  * @param paintEvent [QPaintEvent *]
  */
void MsaRulerWidget::paintEvent(QPaintEvent * /* paintEvent */)
{
    if (msaView_ == nullptr || msaView_->msa() == nullptr)
        return;

    int columnWidth = msaView_->charWidth();
    int horizScrollPos = msaView_->horizontalScrollBar()->value();
    int offset = -horizScrollPos % columnWidth + msaView_->renderXShift();
    int startColumn = (horizScrollPos / columnWidth) + 1;  // Convert to 1-based msa indices
    int msaLength = msaView_->msa()->length();
    int halfColumnWidth = columnWidth / 2.;
    QFontMetrics fontMetrics(font());
    int tickHeight = this->tickHeight();
    int halfTickHeight = tickHeight / 2.;
    int halfTickXShift = unitsPerLabel_ / 2 * columnWidth;

    QStyleOption option;
    option.init(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
    painter.setFont(font());
    painter.setPen(palette().color(QPalette::Text));

    QPen pen = painter.pen();
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidth(qMax(.5, qMin(3., static_cast<qreal>(qRound(msaView_->zoom())))));
    painter.setPen(pen);

    int x = offset;
    int column = 0;
    if (startColumn == 1)
    {
        // Special case: always draw 1 at the very left hand side of alignment
        painter.drawText(x, fontMetrics.ascent(), "1");
        painter.drawLine(x + halfColumnWidth, height() - bottomVerticalPadding_, x + halfColumnWidth, height() - bottomVerticalPadding_ - tickHeight);

        if (unitsPerLabel_ / 2. <= msaLength)
            painter.drawLine(x + halfColumnWidth + halfTickXShift - columnWidth, height() - bottomVerticalPadding_, x + halfColumnWidth + halfTickXShift - columnWidth, height() - bottomVerticalPadding_ - halfTickHeight);

        x += (unitsPerLabel_ - 1) * columnWidth;
        column = unitsPerLabel_;
    }
    else
    {
        column = (static_cast<int>(startColumn / unitsPerLabel_)) * unitsPerLabel_;
        x += (column - startColumn) * columnWidth;
    }

    for (int w = width(); x < w && column < msaLength; column += unitsPerLabel_)
    {
        painter.drawText(x, fontMetrics.ascent(), QString::number(column));
        painter.drawLine(x + halfColumnWidth, height() - bottomVerticalPadding_, x + halfColumnWidth, height() - bottomVerticalPadding_ - tickHeight);

        // Only render the half tick mark if it is still within the msa bounds
        if (column + (unitsPerLabel_ / 2.) <= msaLength)
            painter.drawLine(x + halfColumnWidth + halfTickXShift, height() - bottomVerticalPadding_, x + halfColumnWidth + halfTickXShift, height() - bottomVerticalPadding_ - halfTickHeight);

        x += unitsPerLabel_ * columnWidth;
    }

    // Now draw where the mouse is within the viewport
    {
        painter.setRenderHints(QPainter::Antialiasing);

        int mouseColumn = msaView_->mouseCursorPoint().x();

        // Draw a filled triangle
        QPolygonF triangle;
        triangle << QPointF((mouseColumn - startColumn) * columnWidth + offset + halfColumnWidth - msaView_->renderXShift() + .5, height() - bottomVerticalPadding_ + .5);
        triangle << QPointF(triangle.last().x() - halfColumnWidth, height() - bottomVerticalPadding_ - tickHeight + .5);
        triangle << QPointF(triangle.first().x() + halfColumnWidth, triangle.last().y());

        pen.setWidth(1);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.setBrush(Qt::black);

        painter.drawPolygon(triangle);
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param point [const QPoint &]
  */
void MsaRulerWidget::onViewportMouseMoved(const QPoint & /* point */)
{
    update();
}

/**
  */
void MsaRulerWidget::resizeFont()
{
    ASSERT(msaView_ != nullptr);

    QFont font = this->font();
    int pointSize = ::fitPointSizeFromPixels(font.family(),
                                             msaView_->charHeight(),
                                             logicalDpiY());
    font.setPointSize(pointSize);
    setFont(font);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns int
  */
int MsaRulerWidget::tickHeight() const
{
    return QFontMetrics(font()).ascent() / 2.;
}
