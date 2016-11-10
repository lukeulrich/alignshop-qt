/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>
#include <QtGui/QWheelEvent>

#include "MsaDataColumnWidget.h"
#include "AbstractMsaView.h"
#include "../models/MsaSubseqTableModel.h"
#include "../gui_misc.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
MsaDataColumnWidget::MsaDataColumnWidget(QWidget *parent)
    : AbstractMsaSideWidget(parent),
      model_(nullptr),
      column_(0)
{
}

/**
  * @param msaView [AbstractMsaView *]
  * @param model [MsaSubseqTableModel *]
  * @param column [const int]
  */
MsaDataColumnWidget::MsaDataColumnWidget(AbstractMsaView *msaView, MsaSubseqTableModel *model, const int column, QWidget *parent)
    : AbstractMsaSideWidget(msaView, parent),
      model_(nullptr),
      column_(0)
{
    setModel(model, column);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param column [const int]
  */
void MsaDataColumnWidget::setColumn(const int column)
{
    column_ = column;
    update();
}

/**
  * @param model [MsaSubseqTableModel *]
  * @param column [const int]
  */
void MsaDataColumnWidget::setModel(MsaSubseqTableModel *model, const int column)
{
    if (model_ != nullptr)
    {
        disconnect(model_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onModelDataChanged(QModelIndex,QModelIndex)));
        disconnect(model_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(update()));
        disconnect(model_, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(update()));
        disconnect(model_, SIGNAL(layoutChanged()), this, SLOT(update()));
    }

    model_ = model;
    column_ = column;

    if (model_ != nullptr)
    {
        connect(model_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onModelDataChanged(QModelIndex,QModelIndex)));
        connect(model_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(update()));
        connect(model_, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(update()));
        connect(model_, SIGNAL(layoutChanged()), SLOT(update()));
    }

    update();
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
void MsaDataColumnWidget::paintEvent(QPainter *painter, const int rowHeight, const double baseline, const int startY, const int startMsaRow)
{
    if (model_ == nullptr)
        return;

    QFontMetrics fontMetrics(font());

    int y = startY;
    int row = startMsaRow - 1;      // Use 0-based indices into the model

    painter->setFont(font());
    painter->setPen(palette().color(QPalette::Text));
    for (int h = height(), nRows = model_->rowCount(); y < h && row < nRows; y += rowHeight, ++row)
    {
        QString label = model_->index(row, column_).data().toString();
        if (label.isEmpty())
            continue;

        label = fontMetrics.elidedText(label, Qt::ElideRight, width());

        // Determine where it is to be placed
        qreal x = 0.;
        if (alignment() & Qt::AlignRight)
            x = width() - fontMetrics.width(label);
        else if (alignment() & Qt::AlignCenter)
            x = (width() - fontMetrics.width(label)) / 2.;

        painter->drawText(x, y + baseline, label);
    }
}

/**
  * @param wheelEvent [WheelEvent *]
  */
void MsaDataColumnWidget::wheelEvent(QWheelEvent *wheelEvent)
{
    if (wheelEvent->orientation() != Qt::Vertical)
        return;

    msaView_->scroll(wheelEvent->orientation(), ::stepsInWheelEvent(wheelEvent));
    wheelEvent->accept();
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param topLeft [const QModelIndex &]
  * @param bottomRight [const QModelIndex &]
  */
void MsaDataColumnWidget::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (msaView_ == nullptr)
        return;

    ASSERT(model_ != nullptr);
    ASSERT(topLeft.model() == model_);
    ASSERT(topLeft == bottomRight);

    Q_UNUSED(bottomRight);

    if (topLeft.column() != column_)
        return;

    int rowHeight = msaView_->charHeight();
    int vertScrollPos = msaView_->verticalScrollBar()->value();
    int topRow = vertScrollPos / rowHeight;
    int bottomRow = (vertScrollPos + height()) / rowHeight;

    if (topLeft.row() >= topRow && topLeft.row() <= bottomRow)
        update();
}
