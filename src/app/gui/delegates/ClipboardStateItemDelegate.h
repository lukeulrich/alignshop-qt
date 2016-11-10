/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CLIPBOARDSTATEITEMDELEGATE_H
#define CLIPBOARDSTATEITEMDELEGATE_H

#include <QtGui/QStyledItemDelegate>

class ClipboardStateItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ClipboardStateItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // CLIPBOARDSTATEITEMDELEGATE_H
