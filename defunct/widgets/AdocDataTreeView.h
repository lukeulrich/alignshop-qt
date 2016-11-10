/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCDATATREEVIEW_H
#define ADOCDATATREEVIEW_H

#include <QtGui/QTreeView>

/**
  * Specific tree view widget for interacting with the AlignShop data tree and document interface.
  *
  * Has key handlers in place for handling keyboard events.
  */
class AdocDataTreeView : public QTreeView
{
public:
    AdocDataTreeView(QWidget *parent = 0);

protected slots:
//    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

protected:
    virtual void keyPressEvent(QKeyEvent *event);

private:
};

#endif // ADOCDATATREEVIEW_H
