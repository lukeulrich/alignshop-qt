/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QEvent>

#include <QtGui/QHeaderView>
#include <QtGui/QKeyEvent>

#include "SingleColumnTableView.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
SingleColumnTableView::SingleColumnTableView(QWidget *parent)
    : QTableView(parent),
      column_(0)
{
    horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
int SingleColumnTableView::column() const
{
    return column_;
}

void SingleColumnTableView::setColumn(const int column)
{
    ASSERT(column >= 0);

    horizontalHeader()->hideSection(column_);
    column_ = column;
    horizontalHeader()->showSection(column_);
}

void SingleColumnTableView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);

    hideAllSections();
    setColumn(column_);
}

/**
  * @param shortcut [const QKeySequence &]
  */
void SingleColumnTableView::setIgnoreShortcutOverride(const QKeySequence &shortcut)
{
    int key = 0;
    for (uint i=0; i< shortcut.count(); ++i)
        key += shortcut[i];

    ignoredShortcuts_ << key;
}

/**
  * @param shortcuts [const QList<QKeySequence> &]
  */
void SingleColumnTableView::setIgnoreShortcutOverrides(const QList<QKeySequence> &shortcuts)
{
    foreach (const QKeySequence &shortcut, shortcuts)
    {
        int key = 0;
        for (uint i=0; i< shortcut.count(); ++i)
            key += shortcut[i];

        ignoredShortcuts_ << key;
    }
}

/**
  * @param key [const char]
  */
void SingleColumnTableView::setIgnoreShortcutKey(const char key)
{
    ignoredShortcutKeys_ << key;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * This method prevents global shortcut keys
  *
  * @param event [QEvent *]
  */
bool SingleColumnTableView::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (ignoredShortcutKeys_.contains(keyEvent->key()))
            event->accept();
        else if (ignoredShortcuts_.contains(keyEvent->modifiers() + keyEvent->key()))
            event->accept();
    }

    return QTableView::event(event);
}

void SingleColumnTableView::hideAllSections()
{
    for (int i=0, z= horizontalHeader()->count(); i<z; ++i)
        horizontalHeader()->hideSection(i);
}
