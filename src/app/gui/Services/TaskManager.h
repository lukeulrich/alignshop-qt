/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QSet>
#include "../../core/global.h"

class ITask;
class TaskTreeNode;
class TaskModel;

/**
  * ASSUMPTION: all tasks will be executed in order. Thus if a task requires a certain number of threads and it is not
  * currently available, no additional tasks will be started until this capacity is available - even if other current/
  * pending tasks can utilize a fewer thread count.
  *
  * TaskManager has 4 groups of tasks:
  * 1) Current: These tasks are currently underway with all or a subset active (running). If a task has many slave tasks
  *             it may not be possible to execute all of these simultaneously.
  * 2) Pending: None of these tasks are currently slotted for execution (yet).
  * 3) Completed: Task groups that have finished successfully
  * 4) Errored: Task groups that have had an error at some point
  *
  * Thus the order of a task is:
  * Pending -> Current -> Completed OR Errored
  *
  * If a task errors at any point, no more related tasks are started, any remaining active jobs are waited to complete,
  * and then the whole task group is move to the errored model.
  *
  * If a task requirements exceed the TaskManager's capacity - this task will be marked as completed and given an error
  * status.
  *
  * ISSUE: What happens when the TaskManager is running full tilt, and the associated TaskModel is cleared?
  *        taskAboutToBeDestroyed is emitted for all tasks and currentTaskRow_ is set to zero. Outstanding jobs will
  *        have to finish being killed before the resources become available. Could be an issue if a task requires a
  *        significant amount of time to kill because no jobs will be displayed as running, yet the task manager will
  *        still be utilizing resources.
  */
class TaskManager : public QObject
{
    Q_OBJECT

public:
    TaskManager(QObject *parent = nullptr);

    void enqueue(TaskTreeNode *task);
    int maxActiveTasks() const;
    int maxThreads() const;
    int nActiveTasks() const;
    int nActiveThreads() const;
    TaskModel *pendingTaskModel() const;
    bool runningAtMax() const;  // If there is no more capacity (within the defined maximums) for executing additional tools simultaneously
    void setMaxActiveTasks(const int maxActiveTasks);
    void setMaxThreads(const int maxThreads);
    TaskModel *taskModel() const;


public Q_SLOTS:
    bool remove(ITask *task);
    void start();
    void stop();


Q_SIGNALS:
    void taskAboutToStart(ITask *task);


private Q_SLOTS:
    // Rationale is to respond to enqueued events rather than after the enqueue method has been called in order to
    // avoid packing the enqueue method with error checking. For example, if enqueue failed because of an incompatible
    // request of sorts, we would then have to check in that method for this case before start'ing the next task. On the
    // other hand, we can define that it will only be added to the pending queue on success and therefore, that is the
    // clearest mechanism for triggering that a task should begin.
    void startNewTasksIfPossible();

    // Respond to the done and error signals to update the active task/thread counts
    void onTaskDone(ITask *task);
    // Error handling may take some special care because if an error applies to a whole batch (e.g. invalid BLAST
    // database) there is no point in attempting to analyze the remaining parts in this batch task.
    void onTaskError(ITask *task);

    void onTaskModelReset();
    void onTaskModelRowsRemoved(const QModelIndex &parent, int start, int end);


private:
    void attachTaskSignals(const TaskTreeNode *taskNode) const;
    // Returns true if there is computational capacity currently available for task
    bool capacityAvailableForTask(const ITask *task) const;
    void detachTaskSignals(const TaskTreeNode *taskNode) const;
    int nAvailableThreads() const;
    // Iterates through currentTaskModel looking for a capacity-compatible task to start, returns 0 if none is found
    TaskTreeNode *nextTaskToStart();
    // Automatically starts task running by calling its start method
    void startTask(TaskTreeNode *task);
    // Special case to check if a tasks thread count exceeds the task manager's available threads
    bool taskExceedsTotalCapacity(const ITask *task) const;
    QModelIndex topLevelIndexFromTaskNode(TaskModel *taskModel, TaskTreeNode *taskNode) const;

    // Appropriately adjusts the active threads/tasks based on task
    void increaseLoadCounts(const ITask *task);
    void decreaseLoadCounts(const ITask *task);

    TaskModel *taskModel_;
    int currentTaskRow_;     // Top-level row position of the currently running task or task to be run next.

    bool enabled_;

    int maxThreads_;
    int maxActiveTasks_;

    // To track the number of tasks and threads that are currently going
    int nActiveTasks_;
    int nActiveThreads_;

    // These tasks have encountered an error while processing and thus should not continue being processed until the
    // error is dealt with.
    QSet<TaskTreeNode *> topLevelTaskNodesWithError_;

    QHash<ITask *, TaskTreeNode *> taskTreeNodeHash_;
};

#endif // TASKMANAGER_H
