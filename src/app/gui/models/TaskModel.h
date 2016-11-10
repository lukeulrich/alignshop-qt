/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QQueue>
#include <QtCore/QString>

#include "AbstractBaseTreeModel.h"
#include "../Services/Tasks/ITask.h"
#include "../Services/Tasks/TaskTreeNode.h"
#include "../../core/global.h"

/**
  * TaskModel maintains a hierarchical collection of TaskTreeNodes and their associated Tasks.
  *
  * The only way to add tasks is via the enqueue method.
  *
  * It is not directly possible to rearrange the task order at this time. :)
  *
  * Because ITask is hierarchical, TaskModel is a tree model.
  */
class TaskModel : public AbstractBaseTreeModel<TaskTreeNode>
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Enums
    enum Columns
    {
        eNameColumn = 0,
        eProgressColumn,
        eTimeColumn,
        eThreadsColumn,
        eStatusColumn,
        eNoteColumn,

        eNumberOfColumns
    };


    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    TaskModel(QObject *parent = nullptr);
    ~TaskModel();


    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool contains(ITask *task) const;
    QVariant data(TaskTreeNode *taskNode, int column, int role = Qt::DisplayRole) const;
    void enqueue(TaskTreeNode *taskNode);

    QModelIndex indexFromTaskNode(TaskTreeNode *taskNode) const;
    QModelIndex topLevelIndex(const QModelIndex &index) const;          // Returns the top level index for the given index
    TaskTreeNode *root() const;

    // Removes and releases the root task from this model (does not own it anymore)
    void takeTopLevelTask(TaskTreeNode *rootTaskNode);
    TaskTreeNode *takeTopLevelTask(const QModelIndex &topLevelIndex);

    // Because only a few tasks (out of potentially hundreds or even more) will be running at any given time, only those
    // specifically requested to be observed are actively monitored for changes. Otherwise, it is possible to have to
    // configure many thousands of signal/slot connections, the majority (95%+) of which would never be utilized
    // efficiently at any given time.
    void unwatch(const QModelIndex &index);
    void watch(const QModelIndex &index);


Q_SIGNALS:
    void taskAboutToBeDestroyed(ITask *task);


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void clear();


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onTaskNameChanged(ITask *task);
    void onTaskNoteChanged(ITask *task);
    void onTaskProgressChanged(ITask *task);
    void onTaskStatusChanged(ITask *task);
    void onTaskThreadsChanged(ITask *task);
    void onTaskTimeRunningChanged(ITask *task);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void connectTaskSignals(ITask *task) const;
    void disconnectTaskSignals(ITask *task) const;

    void recurseTeardownNode(TaskTreeNode *node);

    // Each of the above private task X changed slots should call this method with the appropriate column to update.
    // By funneling all changes to this method, it consolidates the code for looking up the relevant QModelIndex and
    // responding appropriately.
    void taskColumnChanged(ITask *task, int column);

    void unwatch(TaskTreeNode *taskNode);
    void watch(TaskTreeNode *taskNode);

    QHash<ITask *, QPersistentModelIndex> watchedTasks_;
};

#endif // TASKMODEL_H
