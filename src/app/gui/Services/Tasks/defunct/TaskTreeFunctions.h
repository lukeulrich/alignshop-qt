/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TASKTREEFUNCTIONS_H
#define TASKTREEFUNCTIONS_H

#include "TaskTreeNode_fwd.h"
#include "ITask.h"

namespace TaskTreeFunctions
{
    bool hasActiveTask(TaskTreeNode *node);
    bool hasUnfinishedTask(TaskTreeNode *node);
    bool contains(TaskTreeNode *node, ITask *task);
    TaskTreeNode *firstNodeWithStatus(TaskTreeNode *node, Ag::TaskStatus status);
}

#endif // TASKTREEFUNCTIONS_H
