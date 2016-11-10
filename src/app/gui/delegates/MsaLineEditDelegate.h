/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSALINEEDITDELEGATE_H
#define MSALINEEDITDELEGATE_H

#include <QtGui/QItemDelegate>

#include "LineEditDelegate.h"
#include "../../core/global.h"

class MsaTableView;

/**
  * MsaLineEditDelegate simply extends LineEditDelegate by providing a scaled version of a line edit widget.
  */
class MsaLineEditDelegate : public LineEditDelegate
{
public:
    // In the one parameter constructor, msaTableView also is the parent
    explicit MsaLineEditDelegate(MsaTableView *msaTableView);
    MsaLineEditDelegate(MsaTableView *msaTableView, QObject *parent);

    //! Returns the widget used to edit the item specified by index for editing
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    MsaTableView *msaTableView_;
};

#endif // MSALINEEDITDELEGATE_H
