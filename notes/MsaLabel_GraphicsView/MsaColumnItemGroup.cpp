/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include <QtGui/QWidget>

#include "MsaColumnItemGroup.h"
#include "MsaSubseqCellItem.h"

#include "../../models/ColumnAdapters/IColumnAdapter.h"
#include "../../../core/Entities/AbstractSeq.h"
#include "../../../core/Msa.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msa [const Msa *]
  * @param columnAdapter [IColumnAdapter *]
  * @param column [const int]
  * @param font [const QFont &]
  * @param baseline [const double]
  * @param rowHeight [const int]
  * @param parentItem [QGraphicsItem *]
  */
MsaColumnItemGroup::MsaColumnItemGroup(const Msa *msa,
                                       IColumnAdapter *columnAdapter,
                                       const int column,
                                       const QFont &font,
                                       const double baseline,
                                       const int rowHeight,
                                       QGraphicsItem *parentItem)
    : QGraphicsItemGroup(parentItem),
      msa_(msa),
      columnAdapter_(columnAdapter),
      column_(column),
      font_(font),
      baseline_(baseline),
      rowHeight_(rowHeight),
      fontMetrics_(font_)
{
    ASSERT(msa_ != nullptr);
    ASSERT(columnAdapter_ != nullptr);
    ASSERT(rowHeight_ > 0);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  *
  */
QRectF MsaColumnItemGroup::boundingRect() const
{
    return QGraphicsItemGroup::boundingRect();
}

/**
  * @param cellItem [const MsaSubseqCellItem *]
  * @returns QRectF
  */
QRectF MsaColumnItemGroup::boundingRect(const MsaSubseqCellItem *cellItem) const
{
    ASSERT(cellItem != nullptr);

    return QRectF(0, 0, fontMetrics_.width(textForItem(cellItem)), rowHeight_);
}

/**
  * @param cellItem [MsaSubseqCellItem *]
  * @param painter [QPainter *]
  * @param option [const QStyleOptionGraphicsItem *]
  * @param widget [QWidget *]
  */
void MsaColumnItemGroup::paint(MsaSubseqCellItem *cellItem, QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget *widget)
{
    // The rationale for this if condition is that the text is dynamically dependent on the items vertical position and
    // if the view extends beyond the msa boundaries, some items will not necessarily correlate to actual data labels.
    //
    // The other possibility is that indeed there is no text for this row, and therefore it is not necessary to bother
    // painting nothing.
//    QString label = QString::number(msaRow(cellItem->scenePos().y()));
    QString label = textForItem(cellItem);
    if (label.isEmpty())
        return;

    label = fontMetrics_.elidedText(label, Qt::ElideRight, widget->width());

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->translate(0, baseline_);
    painter->setFont(font_);
    painter->drawText(0, 0, label);
    painter->restore();
}

/**
  * @returns int
  */
int MsaColumnItemGroup::rowHeight() const
{
    return rowHeight_;
}

/**
  * @param baseline [const double]
  */
void MsaColumnItemGroup::setBaseline(const double baseline)
{
    baseline_ = baseline;
    update();
}

/**
  * @param font [const QFont &]
  */
void MsaColumnItemGroup::setFont(const QFont &font)
{
    font_ = font;
    fontMetrics_ = QFontMetrics(font_);
    prepareGeometryChange();
    // Because the setting the font applies to all child items and will thus change its bounding rect, it is necessary
    // to inform the scene that the geometry of each child item will be changing.
    for (int i=0, z=cellItems_.size(); i<z; ++i)
        cellItems_[i]->prepareGeometryChange();
}

/**
  * @param rowHeight [const int]
  */
void MsaColumnItemGroup::setRowHeight(const int rowHeight)
{
    ASSERT(rowHeight > 0);
    rowHeight_ = rowHeight;

    prepareGeometryChange();
    for (int i=1, z=cellItems_.size(); i<z; ++i)
    {
        cellItems_[i]->prepareGeometryChange();
        cellItems_[i]->setY(i * rowHeight_);
    }
}

/**
  * @param nRows [const int]
  */
void MsaColumnItemGroup::setRows(const int nRows)
{
    ASSERT(nRows >= 0);

    if (nRows > cellItems_.size())
    {
        cellItems_.reserve(nRows);
        for (int i=cellItems_.size(); i< nRows; ++i)
        {
            MsaSubseqCellItem *cellItem = new MsaSubseqCellItem(this);
            // It is vital to add this item to the group *before* setting its y position, otherwise, per the docs:
            // "its position and transformation relative to the scene will stay intact", which means if the scene has
            // been scrolled at all, things will be out of sync. We want to assign its y position relative to this
            // group.
            addToGroup(cellItem);
            cellItem->setY(i * rowHeight_);
            cellItems_ << cellItem;
        }

//        qDebug() << "--GROW---------------------------------------------";
//        for (int i=0; i< cellItems_.size(); ++i)
//            qDebug() << i << "Y:" << cellItems_.at(i)->y() << "Label:" << textForItem(cellItems_.at(i));
    }
    else if (nRows < cellItems_.size())
    {
        for (int i=nRows, z = cellItems_.size(); i<z; ++i)
        {
            MsaSubseqCellItem *cellItem = cellItems_.at(i);

            // First remove from the scene
            cellItem->scene()->removeItem(cellItem);

            delete cellItem;
        }

        cellItems_.resize(nRows);

//        qDebug() << "--SHRINK---------------------------------------------";
//        for (int i=0; i< cellItems_.size(); ++i)
//            qDebug() << i << "Y:" << cellItems_.at(i)->y() << "Label:" << textForItem(cellItems_.at(i));
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param yPos [const double]
  * @returns int
  */
int MsaColumnItemGroup::msaRow(const double yPos) const
{
    return 1 + yPos / rowHeight_;
}

/**
  * @param cellItem [const MsaSubseqCellItem *]
  * @returns QString
  */
QString MsaColumnItemGroup::textForItem(const MsaSubseqCellItem *cellItem) const
{
    return textForRow(msaRow(cellItem->scenePos().y()));
}

/**
  * @param row [const int]
  * @returns QString
  */
QString MsaColumnItemGroup::textForRow(const int row) const
{
    if (msa_->isValidRow(row))
        return columnAdapter_->data(msa_->at(row)->seqEntity_, column_).toString();

    return QString();
}
