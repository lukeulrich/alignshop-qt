/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "TaskModel.h"
#include "../Services/Tasks/Task.h"
#include "../../core/PointerTreeNode.h"
#include "../../core/macros.h"
#include "../../core/misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
TaskModel::TaskModel(QObject *parent)
    : AbstractBaseTreeModel<TaskTreeNode>(parent)
{
    root_ = new TaskTreeNode(new Task(Ag::Group, "Root"));
}

/**
  */
TaskModel::~TaskModel()
{
    clear();
    delete root_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int TaskModel::columnCount(const QModelIndex & /* parent */) const
{
    return eNumberOfColumns;
}

/**
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    return data(nodeFromIndex(index), index.column(), role);
}

/**
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const
{
    TaskTreeNode *taskNode = nodeFromIndex(index);
    if (taskNode == nullptr)
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant TaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    // Horizontal orientation
    ASSERT(orientation == Qt::Horizontal);
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
    case eNameColumn:
        return "Name";
    case eProgressColumn:
        return "Progress";
    case eTimeColumn:
        return "Time";
    case eThreadsColumn:
        return "Threads";
    case eStatusColumn:
        return "Status";
    case eNoteColumn:
        return "Note";

    default:
        return QVariant();
    }
}

/**
  * @param row [int]
  * @param count [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool TaskModel::removeRows(int row, int count, const QModelIndex &parent)
{
    TaskTreeNode *parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr)
        return false;

    // Check that the row is valid
    ASSERT(row >= 0 && row < parentNode->childCount());
    ASSERT(row + count <= parentNode->childCount());

    if (count == 0)
        return true;

    if (!parentNode->isRoot() && parentNode->childCount() == count)
    {
        return removeRows(parent.row(), 1, parent.parent());

        // Since all the parents children are being taken, go ahead and remove the parent node
        ASSERT(row == 0);
        beginRemoveRows(parent.parent(), parent.row(), 1);
        recurseTeardownNode(static_cast<TaskTreeNode *>(parentNode->parent()));
        endRemoveRows();
    }
    else
    {
        beginRemoveRows(parent, row, row + count - 1);
        for (int i=0; i< count; ++i)
            recurseTeardownNode(static_cast<TaskTreeNode *>(parentNode->childAt(row + i)));
        parentNode->removeChildren(row, count);
        endRemoveRows();
    }

    return true;
}

/**
  * @param task [ITask *]
  * @returns bool
  */
bool TaskModel::contains(ITask *task) const
{
    return root_->contains(task);
}

/**
  * @param taskNode [TaskTreeNode *]
  * @param column [int]
  * @param role [int]
  */
QVariant TaskModel::data(TaskTreeNode *taskNode, int column, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (taskNode == nullptr ||
        taskNode == root_)
    {
        return QVariant();
    }

    ITask *task = taskNode->task();
    ASSERT(task != nullptr);
    if (column == eNameColumn)
        return task->name();

    if (!taskNode->isLeaf())
        return QVariant();

    switch (column)
    {
    case eProgressColumn:
        if (task->status() != Ag::NotStarted && task->progress() > 0)
            return QString::number(static_cast<int>(task->progress() * 100.)) + "%";
        return "-";
    case eTimeColumn:
        if (task->status() != Ag::NotStarted && task->progress() > 0)
            return ::formatTimeRunning(task->timeRunning());
        return "-";
    case eThreadsColumn:
        return task->nThreads();
    case eStatusColumn:
        switch (task->status())
        {
        case Ag::NotStarted:
            return "Not started";
        case Ag::Starting:
            return "Starting";
        case Ag::Running:
            return "Running";
        case Ag::Paused:
            return "Paused";
        case Ag::Finished:
            return "Done";
        case Ag::Error:
            return "Error";
        case Ag::Killed:
            return "Killed";

        default:
            return QVariant();
        }
    case eNoteColumn:
        return task->note();

    default:
        return QVariant();
    }
}

/**
  * Takes ownership of task.
  *
  * @param task [TaskTreeNode *]
  */
void TaskModel::enqueue(TaskTreeNode *taskNode)
{
    ASSERT(taskNode != nullptr);
    ASSERT(taskNode->task() != nullptr);

    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    root_->appendChild(taskNode);
    endInsertRows();
}

/**
  * @param taskNode [TaskTreeNode *]
  * @returns QModelIndex
  */
QModelIndex TaskModel::indexFromTaskNode(TaskTreeNode *taskNode) const
{
    if (taskNode == nullptr)
        return QModelIndex();

    ASSERT(taskNode->isDescendantOf(root_));

    return createIndex(taskNode->row(), 0, taskNode);
}

/**
  * @param index [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex TaskModel::topLevelIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ASSERT(index.model() == this);
    QModelIndex result = index;
    while (result.parent().isValid())
        result = result.parent();

    return result;
}

/**
  * @returns TaskTreeNode *
  */
TaskTreeNode *TaskModel::root() const
{
    return root_;
}

/**
  * Because we are taking rootTaskNode from this model, there is no need for a return value. Ownership is passed to the
  * owner.
  *
  * @param rootTaskNode [TaskTreeNode *]
  * @see takeTopLevelTask(const QModelIndex &)
  */
void TaskModel::takeTopLevelTask(TaskTreeNode *rootTaskNode)
{
    ASSERT(rootTaskNode != nullptr);
    ASSERT(root_->childAt(rootTaskNode->row()) == rootTaskNode);

    int row = rootTaskNode->row();

    beginRemoveRows(QModelIndex(), row, row);
    root_->takeChildAt(row);
    endRemoveRows();

    if (watchedTasks_.contains(rootTaskNode->task()))
        unwatch(watchedTasks_.value(rootTaskNode->task()));
}

/**
  * Unlike the other method with this name, this returns the corresponding TaskTreeNode pointer from a model index. If
  * there is no valid pointer for this index, a null pointer is returned.
  *
  * @param topLevelIndex [const QModelIndex &]
  * @returns TaskTreeNode *
  */
TaskTreeNode *TaskModel::takeTopLevelTask(const QModelIndex &topLevelIndex)
{
    TaskTreeNode *taskNode = nodeFromIndex(topLevelIndex);
    if (taskNode != nullptr && topLevelIndex.parent() == QModelIndex())
    {
        int row = topLevelIndex.row();
        beginRemoveRows(QModelIndex(), row, row);
        root_->takeChildAt(row);
        endRemoveRows();

        if (watchedTasks_.contains(taskNode->task()))
            unwatch(watchedTasks_.value(taskNode->task()));

        return taskNode;
    }

    return nullptr;
}

/**
  * @param index [const QModelIndex &]
  */
void TaskModel::unwatch(const QModelIndex &index)
{
    unwatch(nodeFromIndex(index));
}

/**
  * @param index [const QModelIndex &]
  */
void TaskModel::watch(const QModelIndex &index)
{
    TaskTreeNode *taskNode = nodeFromIndex(index);
    watch(taskNode);

    watchedTasks_.insert(taskNode->task(), index);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void TaskModel::clear()
{
    recurseTeardownNode(root_);

    beginResetModel();
    root_->removeChildren();
    // No need to manually unwatch all entries in the watchedTasks_ hash because they will be appropriately disconnected
    // when they are deleted from the above removeChildren method.
    watchedTasks_.clear();
    endResetModel();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param task [ITask *]
  */
void TaskModel::onTaskNameChanged(ITask *task)
{
    taskColumnChanged(task, eNameColumn);
}

/**
  * @param task [ITask *]
  */
void TaskModel::onTaskNoteChanged(ITask *task)
{
    taskColumnChanged(task, eNoteColumn);
}

/**
  * @param task [ITask *]
  */
void TaskModel::onTaskProgressChanged(ITask *task)
{
    taskColumnChanged(task, eProgressColumn);
}

/**
  * @param task [ITask *]
  */
void TaskModel::onTaskStatusChanged(ITask *task)
{
    taskColumnChanged(task, eStatusColumn);
}

/**
  * @param task [ITask *]
  */
void TaskModel::onTaskThreadsChanged(ITask *task)
{
    taskColumnChanged(task, eThreadsColumn);
}

/**
  * @param task [ITask *]
  */
void TaskModel::onTaskTimeRunningChanged(ITask *task)
{
    taskColumnChanged(task, eTimeColumn);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param task [ITask *]
  */
void TaskModel::connectTaskSignals(ITask *task) const
{
    ASSERT(task != nullptr);

    connect(task, SIGNAL(nameChanged(ITask*)), SLOT(onTaskNameChanged(ITask*)), Qt::UniqueConnection);
    connect(task, SIGNAL(noteChanged(ITask*)), SLOT(onTaskNoteChanged(ITask*)), Qt::UniqueConnection);
    connect(task, SIGNAL(progressChanged(ITask*)), SLOT(onTaskProgressChanged(ITask*)), Qt::UniqueConnection);
    connect(task, SIGNAL(statusChanged(ITask*)), SLOT(onTaskStatusChanged(ITask*)), Qt::UniqueConnection);
    connect(task, SIGNAL(timeRunningChanged(ITask*)), SLOT(onTaskTimeRunningChanged(ITask*)), Qt::UniqueConnection);
    connect(task, SIGNAL(threadsChanged(ITask*)), SLOT(onTaskThreadsChanged(ITask*)), Qt::UniqueConnection);
}

/**
  * @param task [ITask *]
  */
void TaskModel::disconnectTaskSignals(ITask *task) const
{
    ASSERT(task != nullptr);

    disconnect(task, SIGNAL(nameChanged(ITask*)), this, SLOT(onTaskNameChanged(ITask*)));
    disconnect(task, SIGNAL(noteChanged(ITask*)), this, SLOT(onTaskNoteChanged(ITask*)));
    disconnect(task, SIGNAL(progressChanged(ITask*)), this, SLOT(onTaskProgressChanged(ITask*)));
    disconnect(task, SIGNAL(statusChanged(ITask*)), this, SLOT(onTaskStatusChanged(ITask*)));
    disconnect(task, SIGNAL(timeRunningChanged(ITask*)), this, SLOT(onTaskTimeRunningChanged(ITask*)));
    disconnect(task, SIGNAL(threadsChanged(ITask*)), this, SLOT(onTaskThreadsChanged(ITask*)));
}

/**
  * Should only be called just before node is about to be removed. Kill any active jobs.
  *
  * @param node [TaskTreeNode *]
  */
void TaskModel::recurseTeardownNode(TaskTreeNode *node)
{
    if (node == nullptr)
        return;

    for (int i=0, z=node->childCount(); i<z; ++i)
        recurseTeardownNode(static_cast<TaskTreeNode *>(node->childAt(i)));

    if (node->task() != nullptr)
    {
        emit taskAboutToBeDestroyed(node->task());
        if (node->task()->isActive())
        {
            // Pass ownership to the task itself
//            node->task()->killAndDeleteLater();
            node->task()->kill();

            // Take the Task pointer away from the the TaskTreeNode since the task itself is now responsible for its
            // own deallocation (see previous line of code).
//            node->take();
        }
    }

    // No need to explicitly free the task nodes here because when the TaskTreeNode is a PointerTreeNode which frees its
    // pointer data (e.g. Task * in this case) automatically when the host node is deleted.
}


/**
  * @param task [ITask *]
  * @param column [int]
  */
void TaskModel::taskColumnChanged(ITask *task, int column)
{
    ASSERT(task != nullptr);

    if (watchedTasks_.contains(task))
    {
        QModelIndex index = watchedTasks_.value(task);
        if (index.isValid())
        {
            QModelIndex changedIndex = index.sibling(index.row(), column);
            emit dataChanged(changedIndex, changedIndex);
        }
        else
        {
            // Technically, I don't know how we can get here if everything is wired properly.
            ASSERT(0);

            // The persistent index associated with this task has been removed - in turn we should assume that the
            // taskNode has been removed. We disconnect the signals because somehow we received them.
            disconnectTaskSignals(task);
            watchedTasks_.remove(task);
        }
    }
}



/**
  * @param taskNode [TaskTreeNode *]
  */
void TaskModel::unwatch(TaskTreeNode *taskNode)
{
    ASSERT(taskNode != nullptr);
    if (taskNode == nullptr)
        return;

    ITask *task = taskNode->task();
    watchedTasks_.remove(task);
    disconnectTaskSignals(task);
}

/**
  * @param taskNode [TaskTreeNode *]
  */
void TaskModel::watch(TaskTreeNode *taskNode)
{
    ASSERT(taskNode != nullptr);
    if (taskNode == nullptr)
        return;

    ITask *task = taskNode->task();
    connectTaskSignals(task);
}
