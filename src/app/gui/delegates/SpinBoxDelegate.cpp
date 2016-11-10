/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QSpinBox>

#include "SpinBoxDelegate.h"
#include "../models/CustomRoles.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [QWidget *]
  * @param option [const QStyleOptionViewItem &]
  * @param index [const QModelIndex &]
  * @returns QWidget *
  */
QWidget *SpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /* option */, const QModelIndex &index) const
{
    QSpinBox *spinBox = new QSpinBox(parent);

    // Attempt to fetch the minimum and maximum values
    bool ok = false;
    int minimum = index.data(CustomRoles::kMinRole).toInt(&ok);
    if (ok)
        spinBox->setMinimum(minimum);

    int maximum = index.data(CustomRoles::kMaxRole).toInt(&ok);
    if (ok)
        spinBox->setMaximum(maximum);

    return spinBox;
}

/**
  * @param editor [QWidget *]
  * @param index [const QModelIndex &]
  */
void SpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->setValue(index.data(Qt::EditRole).toInt());
}

/**
  * @param editor [QWidget *]
  * @param model [QAbstractItemModel *]
  * @param index [const QModelIndex &]
  */
void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->interpretText();
    model->setData(index, spinBox->value());

}

/**
  * @param editor [QWidget *]
  * @param option [const QStyleOptionViewItem &]
  * @param index [const QModelIndex &]
  */
void SpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}
