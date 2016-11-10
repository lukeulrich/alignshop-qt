/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TASKTREENODE_FWD_H
#define TASKTREENODE_FWD_H

class ITask;

template<typename T> class PointerTreeNode;
typedef PointerTreeNode<ITask> TaskTreeNode;

template<typename T> class QVector;
typedef QVector<TaskTreeNode *> TaskTreeNodeVector;     // Assume pointer

#endif // TASKTREENODE_FWD_H
