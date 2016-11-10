/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QLineEdit>
#include <QtGui/QPainter>

#include "LineEditDelegate.h"
#include "../models/CustomRoles.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param column [int]
  * @param parent [QObject *]
  */
LineEditDelegate::LineEditDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * @param parent [QWidget *]
  * @param option [const QStyleOptionViewItem]
  * @param index [const QModelIndex &]
  * @returns QWidget *
  */
QWidget *LineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const
{
    return new QLineEdit(parent);
}

/**
  * After setting the editor data, all the text data is selected.
  *
  * @param editor [QWidget *]
  * @param index [const QModelIndex &]
  */
void LineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    lineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
    lineEdit->selectAll();
}

/**
  */
void LineEditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.model()->data(index, CustomRoles::kIsCutRole).toBool() == false)
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // Otherwise, we have a cut item to paint
    painter->save();
    painter->fillRect(option.rect, QColor(204, 216, 235));
    painter->setOpacity(.5);
    QStyledItemDelegate::paint(painter, option, index);
    painter->restore();
}
