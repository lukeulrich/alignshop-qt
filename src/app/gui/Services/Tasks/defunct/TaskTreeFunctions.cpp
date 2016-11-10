/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "TaskTreeFunctions.h"
#include "ITask.h"

#include "../../../core/PointerTreeNode.h"
#include "../../../core/macros.h"


/**
  * @param node [TaskTreeNode *]
  * @returns bool
  */
bool TaskTreeFunctions::hasActiveTask(TaskTreeNode *node)
{
    if (node == nullptr)
        return false;

    TaskTreeNode::ConstIterator it = node;
    for (TaskTreeNode::ConstIterator end = node->nextAscendant(); it != end; ++it)
        if ((*it)->isActive())
            return true;

    return false;
}

/**
  * @param node [TaskTreeNode *]
  * @returns bool
  */
bool TaskTreeFunctions::hasUnfinishedTask(TaskTreeNode *node)
{
    if (node == nullptr)
        return false;

    TaskTreeNode::ConstIterator it = node;
    for (TaskTreeNode::ConstIterator end = node->nextAscendant(); it != end; ++it)
        if (!(*it)->isOver())
            return true;

    return false;
}

/**
  * @param node [TaskTreeNode *]
  * @param task [ITask *]
  * @returns bool
  */
bool TaskTreeFunctions::contains(TaskTreeNode *node, ITask *task)
{
    if (node == nullptr)
        return false;

    TaskTreeNode::ConstIterator it = node;
    for (TaskTreeNode::ConstIterator end = node->nextAscendant(); it != end; ++it)
        if (it->data_.data() == task)
            return true;

    return false;
}

/**
  * @param node [TaskTreeNode *]
  * @param status [Ag::TaskStatus]
  * @returns TaskTreeNode *
  */
TaskTreeNode *TaskTreeFunctions::firstNodeWithStatus(TaskTreeNode *node, Ag::TaskStatus status)
{
    if (node == nullptr)
        return nullptr;

    TaskTreeNode::ConstIterator it = node;
    for (TaskTreeNode::ConstIterator end = node->nextAscendant(); it != end; ++it)
        if ((*it)->status() == status)
            return const_cast<TaskTreeNode *>(static_cast<const TaskTreeNode *>(it.node()));

    return nullptr;
}
