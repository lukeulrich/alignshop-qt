/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QDropEvent>
#include <QtGui/QKeyEvent>

#include "AdocTreeView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
AdocTreeView::AdocTreeView(QWidget *parent) : QTreeView(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Due to a bug in the Qt codebase (6679), it is necessary to
  *
  * @param event [QDropEvent *]
  */
void AdocTreeView::dropEvent(QDropEvent *event)
{
    QTreeView::dropEvent(event);
    event->setDropAction(Qt::IgnoreAction);
}

/**
  * @param event [QKeyEvent *]
  */
void AdocTreeView::keyPressEvent(QKeyEvent *event)
{
    if (model() != nullptr)
    {
        if (event->matches(QKeySequence::Cut))
        {
            emit cut(selectionModel()->selection());
            event->accept();
            return;
        }
        if (event->matches(QKeySequence::Paste))
        {
            emit pasteTo(currentIndex());
            event->accept();
            return;
        }

        switch (event->key())
        {
        case Qt::Key_Escape:
            emit clearCut();
            event->accept();
            return;

        case Qt::Key_Delete:
            if (model() == nullptr)
                break;

            emit deleteSelection(selectionModel()->selection());
//            if (model() != nullptr && currentIndex().isValid())
//                model()->removeRow(currentIndex().row(), currentIndex().parent());

            // Do not call the parent class keyPressEvent since we have handled it here. Possible side effects if we did not
            // return: user may have AnyKeyPressed as an EditTrigger which would then trigger an editing event on a
            // neighboring item.
            event->accept();
            return;

        default:
            break;
        }
    }

    QTreeView::keyPressEvent(event);
}
