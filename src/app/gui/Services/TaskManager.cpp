/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "TaskManager.h"
#include "../models/TaskModel.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
TaskManager::TaskManager(QObject *parent)
    : QObject(parent),
      currentTaskRow_(0),
      enabled_(true),
      maxThreads_(2),
      maxActiveTasks_(0),
      nActiveTasks_(0),
      nActiveThreads_(0)
{
    taskModel_ = new TaskModel(this);

    // Note the queued connection so that when we enqueue a task
    connect(taskModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(startNewTasksIfPossible()), Qt::QueuedConnection);
    connect(taskModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(onTaskModelRowsRemoved(QModelIndex,int,int)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Can only enqueue root tasks. Takes ownership of the task.
  *
  * @param taskNode [TaskTreeNode *]
  */
void TaskManager::enqueue(TaskTreeNode *taskNode)
{
    ASSERT(taskNode != nullptr);
    ASSERT(taskNode->isRoot());
    ASSERT(taskNode->hasTaskWithStatus(Ag::NotStarted));

    // TODO: Maybe some other checks here?
    //
    // Decided not to reject any job that exceeds the total capacity because it is possible that the configuration will
    // be changed by the time this job is started.

    taskModel_->enqueue(taskNode);
}

/**
  * @returns int
  */
int TaskManager::maxActiveTasks() const
{
    return maxActiveTasks_;
}

/**
  * @returns int
  */
int TaskManager::maxThreads() const
{
    return maxThreads_;
}

/**
  * @returns int
  */
int TaskManager::nActiveTasks() const
{
    return nActiveTasks_;
}

/**
  * @returns int
  */
int TaskManager::nActiveThreads() const
{
    return nActiveThreads_;
}

/**
  * @returns bool
  */
bool TaskManager::runningAtMax() const
{
    // At first glance, it might appear that it should not ever be possible to exceed the maxActiveTasks or
    // maxActiveThreads; however, this is certainly possible if a user initially sets a high capacity, initiates several
    // long-running tasks, and then reduces the capacity below what is currently running.
    //
    // Consequently, we do not assert that nActiveTasks is <= maxActiveTasks or nActiveThreads <= maxThreads; and the
    // test for running at max utilizes the greater than or equal comparison operator.
    //
    // If maxActiveTasks is 0, then there is no task limit
    if (maxActiveTasks_ != 0)
        return nActiveTasks_ >= maxActiveTasks_ || nActiveThreads_ >= maxThreads_;
    else
        return nActiveThreads_ >= maxThreads_;
}

/**
  * @param maxActiveTasks [const int]
  */
void TaskManager::setMaxActiveTasks(const int maxActiveTasks)
{
    ASSERT(maxActiveTasks >= 0);
    maxActiveTasks_ = maxActiveTasks;
}

/**
  * @param maxThreads [const int]
  */
void TaskManager::setMaxThreads(const int maxThreads)
{
    ASSERT(maxThreads >= 0);
    maxThreads_ = maxThreads;
}

/**
  * @returns TaskModel *
  */
TaskModel *TaskManager::taskModel() const
{
    return taskModel_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param task [ITask *]
  * @returns bool
  */
bool TaskManager::remove(ITask *task)
{
    if (task == nullptr)
        return false;

    TaskTreeNode *taskNode = taskModel_->root()->nodeForTask(task);
    if (taskNode == nullptr)
        return false;

    QModelIndex index = taskModel_->indexFromTaskNode(taskNode);
    if (!index.isValid())
        return false;

    return taskModel_->removeRows(index.row(), 1, index.parent());
}

/**
  */
void TaskManager::start()
{
    if (enabled_)
        return;

    enabled_ = true;

    startNewTasksIfPossible();
}

/**
  */
void TaskManager::stop()
{
    enabled_ = false;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param task [ITask *]
  */
void TaskManager::onTaskDone(ITask *task)
{
    ASSERT(taskTreeNodeHash_.contains(task));
    decreaseLoadCounts(task);

    TaskTreeNode *taskNode = taskTreeNodeHash_.take(task);
    ASSERT(taskModel_->contains(taskNode->task()));
    taskModel_->unwatch(taskModel_->indexFromTaskNode(taskNode));
    detachTaskSignals(taskNode);

    QModelIndex topLevelIndex = topLevelIndexFromTaskNode(taskModel_, taskNode);

    TaskTreeNode *topLevelTaskNode = taskModel_->nodeFromIndex(topLevelIndex);
    if (topLevelTaskNodesWithError_.contains(topLevelTaskNode))
    {
        // This top level task has had an error at some point, but task has completed successfully. Check for any other
        // tasks still running.
        if (!topLevelTaskNode->anyTaskActive())
            // Remove from the rootTasksWithError structure since it is no longer in the current task model
            topLevelTaskNodesWithError_.remove(topLevelTaskNode);
    }

    startNewTasksIfPossible();
}

/**
  * @param task [ITask *]
  */
void TaskManager::onTaskError(ITask *task)
{
    ASSERT(taskTreeNodeHash_.contains(task));
    decreaseLoadCounts(task);

    TaskTreeNode *taskNode = taskTreeNodeHash_.take(task);
    ASSERT(taskNode->task() == task);
    ASSERT(taskModel_->contains(task));
    taskModel_->unwatch(taskModel_->indexFromTaskNode(taskNode));
    detachTaskSignals(taskNode);

    QModelIndex topLevelIndex = topLevelIndexFromTaskNode(taskModel_, taskNode);

    // What if other jobs in this potential task group are still running?
    TaskTreeNode *topLevelTaskNode = taskModel_->nodeFromIndex(topLevelIndex);
    if (!topLevelTaskNode->anyTaskActive())
    {
        // It is possible to have 2 jobs running and the first causes an error, and then the second errors out. In this
        // case, the topLevelTaskNodesWithError_ would have an entry.
        if (topLevelTaskNodesWithError_.contains(topLevelTaskNode))
            topLevelTaskNodesWithError_.remove(topLevelTaskNode);
    }
    else    // Remember the root task that has a problem and deal with when the other running tasks exit
    {
        topLevelTaskNodesWithError_ << topLevelTaskNode;
    }

    // Just because this task failed does not mean that there are not other tasks we can process
    startNewTasksIfPossible();
}

/**
  */
void TaskManager::onTaskModelReset()
{
    currentTaskRow_ = 0;
    topLevelTaskNodesWithError_.clear();

}

/**
  * Update the currentTaskRow_ as necessary.
  *
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void TaskManager::onTaskModelRowsRemoved(const QModelIndex &parent, int start, int /* end */)
{
    TaskTreeNode *taskTreeNode = taskModel_->nodeFromIndex(parent);
    if (taskTreeNode == nullptr)
        return;

    // Skip any non-top level nodes
    if (parent.isValid())
        return;

    if (start <= currentTaskRow_)
        currentTaskRow_ = start;
}


/**
  */
void TaskManager::startNewTasksIfPossible()
{
    if (!enabled_)
        return;

    while (!runningAtMax())
    {
        TaskTreeNode *nextTaskNode = nextTaskToStart();
        if (nextTaskNode == nullptr)
            break;

        // Can only start tasks that are leaves
        ASSERT(nextTaskNode->isLeaf() && nextTaskNode->task()->type() == Ag::Leaf);

        // Check that the task can be executed at any time given the total capacity
        ITask *nextTask = nextTaskNode->task();
        if (taskExceedsTotalCapacity(nextTask))
        {
            nextTask->setStatus(Ag::Error);
            nextTask->setNote("Task requires more threads than are available. Please check your configuration.");

            // To prevent an infinite loop, add to the topLevelNodes with error
            QModelIndex topLevelTaskIndex = topLevelIndexFromTaskNode(taskModel_, nextTaskNode);
            topLevelTaskNodesWithError_ << taskModel_->nodeFromIndex(topLevelTaskIndex);
            continue;
        }

        // Check that we have enough capacity free to run this task now
        if (!capacityAvailableForTask(nextTask))
            break;

        // Yep - we have capacity to run the current job
        startTask(nextTaskNode);
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param taskNode [const TaskTreeNode *]
  */
void TaskManager::attachTaskSignals(const TaskTreeNode *taskNode) const
{
    ASSERT(taskNode != nullptr);
    ASSERT(taskNode->task() != nullptr);

    ITask *task = taskNode->task();
    connect(task, SIGNAL(done(ITask*)), SLOT(onTaskDone(ITask*)), Qt::UniqueConnection);
    connect(task, SIGNAL(error(ITask*)), SLOT(onTaskError(ITask*)), Qt::UniqueConnection);
}

/**
  * @param task [const ITask *]
  * @returns bool
  */
bool TaskManager::capacityAvailableForTask(const ITask *task) const
{
    ASSERT(task != nullptr);

    return task->minThreads() <= nAvailableThreads() &&
            (maxActiveTasks_ == 0 || nActiveTasks_ < maxActiveTasks_);
}

/**
  * @param taskNode [const TaskTreeNode *]
  */
void TaskManager::detachTaskSignals(const TaskTreeNode *taskNode) const
{
    ASSERT(taskNode != nullptr);
    ASSERT(taskNode->task() != nullptr);

    ITask *task = taskNode->task();
    disconnect(task, SIGNAL(done(ITask*)), this, SLOT(onTaskDone(ITask*)));
    disconnect(task, SIGNAL(error(ITask*)), this, SLOT(onTaskError(ITask*)));
}

/**
  * @returns int
  */
int TaskManager::nAvailableThreads() const
{
    return qMax(0, maxThreads_ - nActiveThreads_);
}

/**
  * Traverse the task model tree from the currentTaskRow_ and find any unstarted tasks.
  *
  * @returns TaskTreeNode *
  */
TaskTreeNode *TaskManager::nextTaskToStart()
{
    for (int z=taskModel_->rowCount(); currentTaskRow_<z; ++currentTaskRow_)
    {
        QModelIndex topLevelTaskIndex = taskModel_->index(currentTaskRow_, 0);
        TaskTreeNode *topTaskNode = taskModel_->nodeFromIndex(topLevelTaskIndex);
        ASSERT(topTaskNode != nullptr);

        // Avoid queueing any additional jobs if they have errored at any point
        if (topLevelTaskNodesWithError_.contains(topTaskNode))
            continue;

        TaskTreeNode *nextTaskNode = topTaskNode->firstLeafWithStatus(Ag::NotStarted);
        if (nextTaskNode != nullptr)
            return nextTaskNode;
    }

    return nullptr;
}

/**
  * @param taskNode [TaskTreeNode *]
  */
void TaskManager::startTask(TaskTreeNode *taskNode)
{
    ASSERT(taskNode != nullptr);
    ASSERT(taskNode->task() != nullptr);
    ASSERT(taskNode->task()->status() == Ag::NotStarted);

    taskModel_->watch(taskModel_->indexFromTaskNode(taskNode));
    attachTaskSignals(taskNode);
    ITask *task = taskNode->task();
    taskTreeNodeHash_.insert(task, taskNode);

    ASSERT(capacityAvailableForTask(task));

    // Assign this task the maximum number of threads possible
    task->setThreads(qMin(task->maxThreads(), nAvailableThreads()));
    task->setStatus(Ag::Starting);

    // Update the number of active threads
    increaseLoadCounts(task);

    emit taskAboutToStart(task);

    // Kick off the task
    task->start();
}

/**
  * @param task [const ITask *]
  * @returns bool
  */
bool TaskManager::taskExceedsTotalCapacity(const ITask *task) const
{
    ASSERT(task != nullptr);
    return task->minThreads() > maxThreads_;
}

/**
  * @param taskNode [TaskTreeNode *]
  * @returns QModelIndex
  */
QModelIndex TaskManager::topLevelIndexFromTaskNode(TaskModel *taskModel, TaskTreeNode *taskNode) const
{
    QModelIndex taskIndex = taskModel->indexFromTaskNode(taskNode);
    ASSERT(taskIndex.isValid());
    return taskModel->topLevelIndex(taskIndex);
}

/**
  * @param task [const ITask *]
  */
void TaskManager::increaseLoadCounts(const ITask *task)
{
    ++nActiveTasks_;
    nActiveThreads_ += task->nThreads();
}

/**
  * @param task [const ITask *]
  */
void TaskManager::decreaseLoadCounts(const ITask *task)
{
    --nActiveTasks_;
    nActiveThreads_ -= task->nThreads();
}
