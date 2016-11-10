/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSASUBSEQCELLITEM_H
#define MSASUBSEQCELLITEM_H

#include <QtGui/QGraphicsItem>
#include "MsaColumnItemGroup.h"
#include "../../../core/global.h"

/**
  * This can be hazardous because they could reparent the item and that would then cause issues.
  */
class MsaSubseqCellItem : public QGraphicsItem
{
public:
    MsaSubseqCellItem(MsaColumnItemGroup *parentGroup = nullptr)
        : QGraphicsItem(), parentGroup_(parentGroup)
    {
    }

    virtual QRectF boundingRect() const
    {
        if (parentGroup_ != nullptr)
            return parentGroup_->boundingRect(this);

        return QRectF();
    }
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        if (parentGroup_ != nullptr)
            parentGroup_->paint(this, painter, option, widget);
    }

private:
    MsaColumnItemGroup *parentGroup_;

    friend class MsaColumnItemGroup;
};

#endif // MSASUBSEQCELLITEM_H
