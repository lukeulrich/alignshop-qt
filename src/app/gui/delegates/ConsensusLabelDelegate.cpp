/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QSpinBox>

#include "ConsensusLabelDelegate.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
ConsensusLabelDelegate::ConsensusLabelDelegate(QObject *parent)
    : SpinBoxDelegate(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
QWidget *ConsensusLabelDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *editor = SpinBoxDelegate::createEditor(parent, option, index);
    if (editor == nullptr)
        return nullptr;

    ASSERT(qobject_cast<QSpinBox *>(editor) != 0);
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->setSuffix("%");
    spinBox->setMinimum(50);
    spinBox->setMaximum(100);
    return spinBox;
}
