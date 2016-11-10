/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSACOLUMNITEMGROUP_H
#define MSACOLUMNITEMGROUP_H

#include <QtCore/QVector>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsItemGroup>
#include "../../../core/global.h"

class Msa;
class MsaSubseqCellItem;
class IColumnAdapter;

/**
  * MsaColumnItemGroup extends a QGraphicsItemGroup with special capabilities targeting
  */
class MsaColumnItemGroup : public QGraphicsItemGroup
{
public:
    MsaColumnItemGroup(const Msa *msa, IColumnAdapter *columnAdapter, const int column, const QFont &font, const double baseline, const int rowHeight, QGraphicsItem *parentItem = nullptr);

    QRectF boundingRect() const;
    QRectF boundingRect(const MsaSubseqCellItem *cellItem) const;
    void paint(MsaSubseqCellItem *cellItem, QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    int rowHeight() const;
    void setBaseline(const double baseline);
    void setFont(const QFont &font);
    void setRowHeight(const int rowHeight);
    void setRows(const int nRows);

private:
    int msaRow(const double yPos) const;
    QString textForItem(const MsaSubseqCellItem *msaSubseqCellItem) const;
    QString textForRow(const int row) const;

    const Msa *msa_;
    IColumnAdapter *columnAdapter_;
    int column_;
    QFont font_;
    double baseline_;
    int rowHeight_;

    QFontMetrics fontMetrics_;
    QVector<MsaSubseqCellItem *> cellItems_;
};

#endif // MSACOLUMNITEMGROUP_H
