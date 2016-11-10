/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREENODE_FWD_H
#define ADOCTREENODE_FWD_H

class AdocNodeData;

template<typename T> class TreeNode;
typedef TreeNode<AdocNodeData> AdocTreeNode;

template<typename T> class QVector;
typedef QVector<AdocTreeNode *> AdocTreeNodeVector;     // Assume pointer

#endif // ADOCTREENODE_FWD_H
