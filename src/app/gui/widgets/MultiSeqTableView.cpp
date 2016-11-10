/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>

#include "MultiSeqTableView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
MultiSeqTableView::MultiSeqTableView(QWidget *parent) : QTableView(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param event [QKeyEvent *]
  */
void MultiSeqTableView::keyPressEvent(QKeyEvent *event)
{
    if (model() != nullptr)
    {
        if (event->matches(QKeySequence::Copy))
        {
            // Push current index into clipboard
            qApp->clipboard()->setText(currentIndex().data().toString());
            event->accept();
            return;
        }
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

        case Qt::Key_Home:
            selectionModel()->clear();
            selectRow(0);
            event->accept();
            return;
        case Qt::Key_End:
            selectionModel()->clear();
            selectRow(model()->rowCount()-1);
            event->accept();
            return;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit enterPressed(currentIndex());
            break;

        case Qt::Key_Delete:
            {
                // ASSUME: select entire rows, contiguous selection
//                ASSERT(selectionMode() == QAbstractItemView::ContiguousSelection);
//                ASSERT(selectionBehavior() == QAbstractItemView::SelectRows);

                if (model() == nullptr)
                    break;

                emit deleteSelection(selectionModel()->selection());

                /*
                QModelIndexList selection = selectionModel()->selectedRows();
                if (selection.isEmpty())
                    break;

                int row = qMin(selection.first().row(), selection.last().row());
                model()->removeRows(row, selection.size());

                // Do not call the parent class keyPressEvent since we have handled it here. Possible side effects if we did not
                // return: user may have AnyKeyPressed as an EditTrigger which would then trigger an editing event on a
                // neighboring item.

                */
                event->accept();
                return;
            }

        default:
            break;
        }
    }

    event->ignore();
    QTableView::keyPressEvent(event);
}
