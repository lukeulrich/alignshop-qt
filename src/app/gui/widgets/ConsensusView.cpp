/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>
#include <QtGui/QScrollBar>

#include "ConsensusView.h"
#include "../models/MsaConsensusModel.h"
#include "../../graphics/TextColorStyle.h"
#include "../../graphics/AbstractTextRenderer.h"
#include "../painting/IRenderEngine.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
ConsensusView::ConsensusView(QWidget *parent)
    : VerticalMsaMarginWidget(parent),
      renderEngine_(nullptr),
      msaConsensusModel_(nullptr)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
IRenderEngine *ConsensusView::renderEngine() const
{
    return renderEngine_;
}

void ConsensusView::setConsensusModel(MsaConsensusModel *msaConsensusModel)
{
    if (msaConsensusModel_ == msaConsensusModel)
        return;

    if (msaConsensusModel_ != nullptr)
    {
        disconnect(msaConsensusModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(refresh()));
        disconnect(msaConsensusModel_, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(refresh()));
        disconnect(msaConsensusModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onConsensusDataChanged(QModelIndex)));
        disconnect(msaConsensusModel_, SIGNAL(modelReset()), this, SLOT(refresh()));
    }
    msaConsensusModel_ = msaConsensusModel;
    if (msaConsensusModel_ != nullptr)
    {
        connect(msaConsensusModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(refresh()));
        connect(msaConsensusModel_, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(refresh()));
        connect(msaConsensusModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onConsensusDataChanged(QModelIndex)));
        connect(msaConsensusModel_, SIGNAL(modelReset()), SLOT(refresh()));
    }
    updateScrollBarRanges();
}

void ConsensusView::setRenderEngine(IRenderEngine *renderEngine)
{
    renderEngine_ = renderEngine;
    updateScrollBarRanges();
    viewport()->update();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
void ConsensusView::updateHeight()
{
    int newHeight = 0;
    if (renderEngine_ != nullptr || msaConsensusModel_ == nullptr)
        newHeight = msaConsensusModel_->rowCount() * renderEngine_->abstractTextRenderer()->height();
    setFixedHeight(newHeight);
}

void ConsensusView::updateScrollBarRanges()
{
    if (renderEngine_ == nullptr || msaConsensusModel_ == nullptr)
    {
        horizontalScrollBar()->setRange(0, 0);
        verticalScrollBar()->setRange(0, 0);
        return;
    }

    QSize size = renderSize();
    horizontalScrollBar()->setRange(0, size.width() - viewport()->width());
    verticalScrollBar()->setRange(0, size.height() - viewport()->height());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
void ConsensusView::paintEvent(QPaintEvent * /* paintEvent */)
{
    if (renderEngine_ == nullptr || msaConsensusModel_ == nullptr)
        return;

    int symbolStringLength = msaConsensusModel_->symbolStringLength();

    // Determine the range of symbols to render; 0-based calcs
    double charWidth = renderEngine_->abstractTextRenderer()->width();
    int left = horizontalScrollBar()->value() / charWidth;
    int right = qMin(symbolStringLength - 1, left + 1 + static_cast<int>(width() / charWidth));
    double xOrigin = -(horizontalScrollBar()->value() - charWidth * left) + renderXShift();

    TextColorStyle textColorStyle;
    QPainter painter(viewport());
    QPointF pointF;
    for (int i=0, z=msaConsensusModel_->rowCount(); i<z; ++i)
    {
        QModelIndex symbolStringIndex = msaConsensusModel_->index(i, MsaConsensusModel::eSymbolStringColumn);
        if (symbolStringIndex.isValid())
        {
            pointF.rx() = xOrigin;
            const QByteArray symbolString = symbolStringIndex.data().toByteArray();
            const char *c = symbolString.constData() + left;
            for (int i=0, x=left; x<= right; ++x, ++i, ++c)
            {
                if (*c != ' ')
                    renderEngine_->drawBlockChar(pointF, *c, textColorStyle, &painter);
                pointF.rx() += charWidth;
            }
        }
        pointF.ry() += renderEngine_->abstractTextRenderer()->height();
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
void ConsensusView::onConsensusDataChanged(const QModelIndex &topLeft)
{
    if (topLeft.column() == MsaConsensusModel::eSymbolStringColumn)
        viewport()->update();
}

void ConsensusView::refresh()
{
    updateHeight();
    updateScrollBarRanges();
    viewport()->update();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
QSize ConsensusView::renderSize() const
{
    if (renderEngine_ == nullptr || msaConsensusModel_ == nullptr)
        return QSize();

    int rows = msaConsensusModel_->rowCount();
    int symbolStringLength = msaConsensusModel_->symbolStringLength();
    return QSize(symbolStringLength * renderEngine_->abstractTextRenderer()->width(),
                 rows * renderEngine_->abstractTextRenderer()->height());
}
