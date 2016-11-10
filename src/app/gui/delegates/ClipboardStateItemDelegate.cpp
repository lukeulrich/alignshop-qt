/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>

#include "ClipboardStateItemDelegate.h"

#include "../models/CustomRoles.h"

#include <QtDebug>

ClipboardStateItemDelegate::ClipboardStateItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void ClipboardStateItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if (index.model()->data(index, CustomRoles::kIsCutRole).toBool())
    {
        painter->fillRect(option.rect, QColor(204, 216, 235));
        painter->setOpacity(.5);
    }
    else
    {
        painter->setOpacity(1);
    }

    QStyledItemDelegate::paint(painter, option, index);
    painter->restore();
}
