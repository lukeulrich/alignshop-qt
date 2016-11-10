/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QLineEdit>
#include <QtGui/QPainter>

#include "MsaLineEditDelegate.h"
#include "../widgets/MsaTableView.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msaTableView [MsaTableView *]
  */
MsaLineEditDelegate::MsaLineEditDelegate(MsaTableView *msaTableView)
    : LineEditDelegate(msaTableView),
      msaTableView_(msaTableView)
{
    ASSERT(msaTableView_ != nullptr);
}

/**
  * @param msaTableView [MsaTableView *]
  * @param parent [QObject *]
  */
MsaLineEditDelegate::MsaLineEditDelegate(MsaTableView *msaTableView, QObject *parent)
    : LineEditDelegate(parent),
      msaTableView_(msaTableView)
{
    ASSERT(msaTableView_ != nullptr);
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
QWidget *MsaLineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setFrame(QFrame::NoFrame);
    lineEdit->setFont(msaTableView_->font());
    return lineEdit;
}

/**
  * Disable anti-aliasing if pointsize is less than 10.
  */
void MsaLineEditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, option.font.pointSize() >= 10);
    LineEditDelegate::paint(painter, option, index);
    painter->restore();
}
