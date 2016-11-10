/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocDataTreeView.h"

#include <QKeyEvent>

AdocDataTreeView::AdocDataTreeView(QWidget *parent) : QTreeView(parent)
{
}

void AdocDataTreeView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Delete:
        if (model() && currentIndex().isValid())
            model()->removeRow(currentIndex().row(), currentIndex().parent());

        // Do not call the parent class keyPressEvent since we have handled it here. Possible side
        // effects if we did not return: user may have AnyKeyPressed as an EditTrigger which would
        // then trigger an editing event on a neighboring item.
        return;
    default:
        break;
    }

    QTreeView::keyPressEvent(event);
}

/*
void AdocDataTreeView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    setUpdatesEnabled(false);
    QTreeView::dataChanged(topLeft, bottomRight);
    sortByColumn(0);
    setUpdatesEnabled(true);
}
*/
