/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "TaskTreeNode.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param task [ITask *]
  */
TaskTreeNode::TaskTreeNode(ITask *task)
    : BasePointerTreeNode<ITask, TaskTreeNode>(task)
{
    ASSERT(task != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool TaskTreeNode::anyTaskActive() const
{
    if (task()->isActive())
        return true;

    for (int i=0, z=children_.size(); i<z; ++i)
    {
        TaskTreeNode *taskNode = static_cast<TaskTreeNode *>(children_.at(i));
        if (taskNode->anyTaskActive())
            return true;
    }

    return false;
}

/**
  * @returns bool
  */
bool TaskTreeNode::allTasksOver() const
{
    if (!task()->isOver())
        return false;

    for (int i=0, z=children_.size(); i<z; ++i)
    {
        TaskTreeNode *taskNode = static_cast<TaskTreeNode *>(children_.at(i));
        if (!taskNode->allTasksOver())
            return false;
    }

    return true;
}

/**
  * @param task [ITask *]
  * @returns bool
  */
bool TaskTreeNode::contains(ITask *task) const
{
    if (this->data_ == task)
        return true;

    for (int i=0, z= children_.size(); i<z; ++i)
    {
        TaskTreeNode *taskNode = static_cast<TaskTreeNode *>(children_.at(i));
        if (taskNode->contains(task))
            return true;
    }

    return false;
}

/**
  * @param status [Ag::TaskStatus]
  * @returns TaskTreeNode *
  */
TaskTreeNode *TaskTreeNode::firstLeafWithStatus(Ag::TaskStatus status) const
{
    if (isLeaf() && task()->type() == Ag::Leaf && task()->status() == status)
        return const_cast<TaskTreeNode *>(this);

    for (int i=0, z= children_.size(); i<z; ++i)
    {
        TaskTreeNode *childTaskNode = static_cast<TaskTreeNode *>(children_.at(i));
        TaskTreeNode *result = childTaskNode->firstLeafWithStatus(status);
        if (result != nullptr)
            return result;
    }

    return nullptr;
}

/**
  * @param status [const Ag::TaskStatus]
  * @returns bool
  */
bool TaskTreeNode::hasTaskWithStatus(const Ag::TaskStatus status) const
{
    if (isLeaf() && task()->type() == Ag::Leaf)
        return task()->status() == status;

    for (int i=0, z= children_.size(); i<z; ++i)
    {
        TaskTreeNode *childTaskNode = static_cast<TaskTreeNode *>(children_.at(i));
        if (childTaskNode->hasTaskWithStatus(status))
            return true;
    }

    return false;
}

/**
  * @param task [ITask *]
  * @returns TaskTreeNode *
  */
TaskTreeNode *TaskTreeNode::nodeForTask(ITask *task) const
{
    if (this->data_ == task)
        return const_cast<TaskTreeNode *>(this);

    for (int i=0, z= children_.size(); i<z; ++i)
    {
        TaskTreeNode *childTaskNode = static_cast<TaskTreeNode *>(children_.at(i));
        TaskTreeNode *result = childTaskNode->nodeForTask(task);
        if (result != nullptr)
            return result;
    }

    return nullptr;
}

/**
  * @returns TaskTreeNode *
  */
TaskTreeNode *TaskTreeNode::rootTaskNode()
{
    TaskTreeNode *taskNode = this;
    while (!taskNode->isRoot())
        taskNode = taskNode->parent();

    return taskNode;
}

/**
  * @returns ITask *
  */
ITask *TaskTreeNode::task() const
{
    return this->data_;
}
