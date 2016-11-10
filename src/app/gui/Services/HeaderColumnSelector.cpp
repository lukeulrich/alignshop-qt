/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QAbstractItemModel>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QAction>

#include "HeaderColumnSelector.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
HeaderColumnSelector::HeaderColumnSelector(QObject *parent)
    : QObject(parent),
      targetHeader_(nullptr)
{
}

/**
  * @param targetHeader [QHeaderView *]
  * @param parent [QObject *]
  */
HeaderColumnSelector::HeaderColumnSelector(QHeaderView *targetHeader, QObject *parent)
    : QObject(parent),
      targetHeader_(nullptr)
{
    setTargetHeader(targetHeader);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QVector<int>
  */
QVector<int> HeaderColumnSelector::defaultColumns() const
{
    return defaultColumns_;
}

/**
  * @paramm defaultColumns [const QVector<int> &]
  */
void HeaderColumnSelector::setDefaultColumns(const QVector<int> &defaultColumns)
{
    defaultColumns_ = defaultColumns;
}

/**
  * @param targetHeader [QHeaderView *]
  */
void HeaderColumnSelector::setTargetHeader(QHeaderView *targetHeader)
{
    ASSERT(targetHeader->orientation() == Qt::Horizontal);
    if (targetHeader_)
        disconnect(targetHeader_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested()));
    targetHeader_ = targetHeader;
    if (targetHeader_ != nullptr)
    {
        targetHeader_->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(targetHeader_, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onContextMenuRequested()));
    }
}

/**
  * @returns QHeaderView *
  */
QHeaderView *HeaderColumnSelector::targetHeader() const
{
    return targetHeader_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void HeaderColumnSelector::onContextMenuRequested()
{
    // Create the popup menu based on the associated target header view
    if (targetHeader_ == nullptr)
        return;

    if (targetHeader_->model() == nullptr)
        return;

    if (targetHeader_->model()->columnCount() == 0)
        return;

    QSet<int> validDefaultColumns;
    QMenu columnMenu;
    QAbstractItemModel *model = targetHeader_->model();
    for (int i=0, z=model->columnCount(); i<z; ++i)
    {
        QAction *newAction = columnMenu.addAction(model->headerData(i, Qt::Horizontal).toString());
        newAction->setCheckable(true);
        newAction->setChecked(!targetHeader_->isSectionHidden(i));
        if (defaultColumns_.contains(i))
            validDefaultColumns << i;
    }

    QAction *restoreDefaults = nullptr;
    if (validDefaultColumns.size() > 0)
    {
        columnMenu.addSeparator();
        restoreDefaults = columnMenu.addAction("Restore Defaults");
    }

    QAction *action = columnMenu.exec(QCursor::pos());
    if (action == nullptr)
        return;

    int actionIndex = columnMenu.actions().indexOf(action);
    ASSERT(actionIndex != -1);
    if (actionIndex < model->columnCount())
    {
        // Hide/show the relevant column
        if (action->isChecked())
            targetHeader_->showSection(actionIndex);
        else
            targetHeader_->hideSection(actionIndex);

        return;
    }

    if (action == restoreDefaults)
    {
        for (int i=0, z=model->columnCount(); i<z; ++i)
        {
            if (validDefaultColumns.contains(i))
                targetHeader_->showSection(i);
            else
                targetHeader_->hideSection(i);
        }

        return;
    }
}
