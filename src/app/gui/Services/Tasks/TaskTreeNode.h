/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TASKTREENODE_H
#define TASKTREENODE_H

#include "../../../core/BasePointerTreeNode.h"
#include "ITask.h"

/**
  * TaskTreeNode is a specialized PointerTreeNode for organizing a hierarchy of Task objects and facilitating various
  * tree queries based on specific task properties.
  *
  * All tree traversal methods are depth-first, pre-order.
  */
class TaskTreeNode : public BasePointerTreeNode<ITask, TaskTreeNode>
{
public:
    // -------------------------------------------------------------------------------------------------
    // Constructor
    TaskTreeNode(ITask *task);                                      //!< Construct ITaskTreeNode from task

    // -------------------------------------------------------------------------------------------------
    // Public methods
    bool anyTaskActive() const;                                     //!< Returns true if this node or any of its children contains an active task
    bool allTasksOver() const;                                      //!< Returns true if this node and all of its children have their tasks in a completed state
    bool contains(ITask *task) const;                               //!< Returns true if this node or any of its children contains task
    //! Returns the first TaskTreeNode pointer that is a leaf and has status or nullptr if none is found
    TaskTreeNode *firstLeafWithStatus(Ag::TaskStatus status) const;
    bool hasTaskWithStatus(const Ag::TaskStatus status) const;      //!< Returns true if this node or any of its children has status
    TaskTreeNode *nodeForTask(ITask *task) const;                   //!< Returns the TaskTreeNode that contains task or nullptr if none is found
    TaskTreeNode *rootTaskNode();                                   //!< Returns the root TaskTreeNode that owns this instance

    ITask *task() const;                                            //!< Helper method that returns the pointer task instance (instead of having to cast the data member every time)

private:
    Q_DISABLE_COPY(TaskTreeNode)                                   //!< Prevent all copying and assignment
};

#endif // TASKTREENODE_H
