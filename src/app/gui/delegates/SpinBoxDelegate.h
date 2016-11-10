/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SPINBOXDELEGATE_H
#define SPINBOXDELEGATE_H

#include <QtGui/QStyledItemDelegate>
#include "../../core/global.h"

class SpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit SpinBoxDelegate(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // SPINBOXDELEGATE_H
