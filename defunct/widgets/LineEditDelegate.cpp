/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "LineEditDelegate.h"

#include <QtGui/QLineEdit>

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
    lineEdit->setText(index.model()->data(index, Qt::DisplayRole).toString());
    lineEdit->selectAll();
}
