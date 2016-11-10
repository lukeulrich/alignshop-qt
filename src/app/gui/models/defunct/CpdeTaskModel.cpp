/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "CpdeTaskModel.h"
#include "TaskModel.h"
#include "../Services/Tasks/TaskTreeNode.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
CpdeTaskModel::CpdeTaskModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    currentTaskModel_ = new TaskModel(this);
    pendingTaskModel_ = new TaskModel(this);
    doneTaskModel_ = new TaskModel(this);
    errorTaskModel_ = new TaskModel(this);

    // -------------------
    // Hook up the signals
    TaskModel *taskModels[4] = {
        currentTaskModel_,
        pendingTaskModel_,
        doneTaskModel_,
        errorTaskModel_
    };

    for (int i=0; i< 4; ++i)
    {
        connect(taskModels[i], SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), SLOT(onTaskModelRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(taskModels[i], SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onTaskModelRowsInserted(QModelIndex,int,int)));
        connect(taskModels[i], SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(onTaskModelRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(taskModels[i], SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(onTaskModelRowsRemoved(QModelIndex,int,int)));
        connect(taskModels[i], SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onTaskModelDataChanged(QModelIndex,QModelIndex)));
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int CpdeTaskModel::columnCount(const QModelIndex & /* parent */) const
{
    return currentTaskModel_->columnCount();
}

/**
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant CpdeTaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.internalPointer() == nullptr)
    {
        // One of the top four
        if (role == Qt::DisplayRole && index.column() == 0)
        {
            switch (index.row())
            {
            case 0:
                return "Current";
            case 1:
                return "Pending";
            case 2:
                return "Done";
            case 3:
                return "Errors";

            default:
                break;
            }
        }

        return QVariant();
    }

    // One of the task model dealios
    TaskTreeNode *taskTreeNode = static_cast<TaskTreeNode *>(index.internalPointer());
    ASSERT(taskTreeNode->isRoot() == false);
    TaskModel *taskModel = modelWithRoot(taskTreeNode->rootTaskNode());
    return taskModel->data(taskTreeNode, index.column(), role);
}

/**
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags CpdeTaskModel::flags(const QModelIndex & /* index */) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant CpdeTaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
        return currentTaskModel_->headerData(section, orientation, role);

    return QVariant();
}

/**
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex CpdeTaskModel::index(int row, int column, const QModelIndex &parent) const
{
    ASSERT(!parent.isValid() || parent.model() == this);

    // 3 cases:
    // 1) parent is invalid = root of this model
    // 2) parent is one of the top 4
    // 3) parent is a subnode within the relevant task model
    if (parent.isValid())
    {
        // Determine if this is one of the top 4
        if (parent.internalPointer() == nullptr)
        {
            // Case 2
            TaskModel *taskModel = modelFromRow(parent.row());
            QModelIndex taskIndex = taskModel->index(row, column, QModelIndex());
            return createIndex(row, column, taskModel->taskNodeFromIndex(taskIndex));
        }

        // Case 3 (virtually the same as that for Task Model)
        TaskTreeNode *parentTaskNode = static_cast<TaskTreeNode *>(parent.internalPointer());
        if (row < 0 ||
            row >= parentTaskNode->childCount() ||
            column < 0 ||
            column >= TaskModel::eNumberOfColumns)
        {
            return QModelIndex();
        }

        return createIndex(row, column, parentTaskNode->childAt(row));
    }

    // Case 1
    if (row >= 0 && row < rowCount() && column >= 0 && column < columnCount())
        // Note the null pointer created by default - this indicates that this index is one of the top 4 beneath
        // the root (current, pending, done, or error)
        return createIndex(row, column);

    return QModelIndex();
}

/**
  * @param child [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex CpdeTaskModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || child.model() != this)
        return QModelIndex();

    if (child.internalPointer() == nullptr)
        // One of the top four
        return QModelIndex();

    // Is it a child of one of the top 4?
    TaskTreeNode *childTaskNode = static_cast<TaskTreeNode *>(child.internalPointer());
    ASSERT(childTaskNode->isRoot() == false);
    TaskTreeNode *parentTaskNode = static_cast<TaskTreeNode *>(childTaskNode->parent());
    if (parentTaskNode->isRoot())
    {
        // This parent is one of the top four
        TaskModel *model = modelWithRoot(parentTaskNode);
        ASSERT(model != nullptr);

        return createIndex(rowFromModel(model), 0);
    }

    // Else, the child is a descendant of one of the tree models
    return createIndex(parentTaskNode->row(), 0, parentTaskNode);
}

/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int CpdeTaskModel::rowCount(const QModelIndex &parent) const
{
    ASSERT(!parent.isValid() || parent.model() == this);

    if (parent.isValid())
    {
        // Determine if this is one of the top 4
        if (parent.internalPointer() == nullptr)
            return modelFromRow(parent.row())->rowCount();

        // This is not from one of the top 4, simply use the count from the corresponding tree node in the index poniter
        ASSERT(parent.internalPointer() != nullptr);
        return static_cast<TaskTreeNode *>(parent.internalPointer())->childCount();
    }

    return 4;
}

/**
  * @returns TaskModel *
  */
TaskModel *CpdeTaskModel::currentTaskModel() const
{
    return currentTaskModel_;
}

/**
  * @returns TaskModel *
  */
TaskModel *CpdeTaskModel::pendingTaskModel() const
{
    return pendingTaskModel_;
}

/**
  * @returns TaskModel *
  */
TaskModel *CpdeTaskModel::doneTaskModel() const
{
    return doneTaskModel_;
}

/**
  * @returns TaskModel *
  */
TaskModel *CpdeTaskModel::errorTaskModel() const
{
    return errorTaskModel_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param begin [const QModelIndex &]
  * @param end [const QModelIndex &]
  */
void CpdeTaskModel::onTaskModelDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    ASSERT(qobject_cast<TaskModel *>(sender()) != 0);
    ASSERT(begin.internalPointer() != nullptr);
    ASSERT(end.internalPointer() != nullptr);

    TaskTreeNode *beginTaskTreeNode = static_cast<TaskTreeNode *>(begin.internalPointer());
    TaskTreeNode *endTaskTreeNode = static_cast<TaskTreeNode *>(end.internalPointer());

    QModelIndex beginIndex = createIndex(begin.row(), begin.column(), beginTaskTreeNode);
    QModelIndex endIndex = createIndex(end.row(), end.column(), endTaskTreeNode);
    emit dataChanged(beginIndex, endIndex);
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void CpdeTaskModel::onTaskModelRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    ASSERT(parent.isValid() == false);
    ASSERT(qobject_cast<TaskModel *>(sender()) != 0);

    Q_UNUSED(parent);

    TaskModel *model = static_cast<TaskModel *>(sender());
    int row = rowFromModel(model);
    ASSERT(row != -1);
    beginInsertRows(index(row, 0), start, end);
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void CpdeTaskModel::onTaskModelRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    ASSERT(parent.isValid() == false);
    ASSERT(qobject_cast<TaskModel *>(sender()) != 0);

    Q_UNUSED(parent);

    TaskModel *model = static_cast<TaskModel *>(sender());
    int row = rowFromModel(model);
    ASSERT(row != -1);
    beginRemoveRows(index(row, 0), start, end);
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void CpdeTaskModel::onTaskModelRowsInserted(const QModelIndex &parent, int /* start */, int /* end */)
{
    ASSERT(parent.isValid() == false);
    ASSERT(qobject_cast<TaskModel *>(sender()) != 0);

    Q_UNUSED(parent);

    endInsertRows();
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void CpdeTaskModel::onTaskModelRowsRemoved(const QModelIndex &parent, int /* start */, int /* end */)
{
    ASSERT(parent.isValid() == false);
    ASSERT(qobject_cast<TaskModel *>(sender()) != 0);

    Q_UNUSED(parent);

    endRemoveRows();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param row [int]
  * @returns TaskModel *
  */
TaskModel *CpdeTaskModel::modelFromRow(int row) const
{
    ASSERT(row >= 0 && row < 4);

    switch (row)
    {
    case 0:
        return currentTaskModel_;
    case 1:
        return pendingTaskModel_;
    case 2:
        return doneTaskModel_;
    case 3:
        return errorTaskModel_;

    default:
        return nullptr;
    }
}

/**
  * @param root [TaskTreeNode *]
  * @returns TaskModel *
  */
TaskModel *CpdeTaskModel::modelWithRoot(TaskTreeNode *rootTaskNode) const
{
    if (rootTaskNode == nullptr)
        return nullptr;

    ASSERT(rootTaskNode->isRoot());
    if (currentTaskModel_->root() == rootTaskNode)
        return currentTaskModel_;

    if (pendingTaskModel_->root() == rootTaskNode)
        return pendingTaskModel_;

    if (doneTaskModel_->root() == rootTaskNode)
        return doneTaskModel_;

    if (errorTaskModel_->root() == rootTaskNode)
        return errorTaskModel_;

    return nullptr;
}

/**
  * @param model [TaskModel *]
  * @returns int
  */
int CpdeTaskModel::rowFromModel(TaskModel *model) const
{
    ASSERT(model != nullptr);

    if (model == currentTaskModel_)
        return 0;
    if (model == pendingTaskModel_)
        return 1;
    if (model == doneTaskModel_)
        return 2;
    if (model == errorTaskModel_)
        return 3;

    ASSERT(0);
    return -1;
}
