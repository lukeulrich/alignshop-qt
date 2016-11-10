/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ITREENODE_H
#define ITREENODE_H

// Does not actually define all interface methods. Rather, it defines those necessary for reconstituting a tree
// using non-templated methods.
class ITreeNode
{
public:
    virtual ~ITreeNode() {}

    virtual void appendChild(ITreeNode *child) = 0;
    virtual int childCount() const = 0;
    virtual ITreeNode *childAt(int row) const = 0;
    virtual bool isRoot() const = 0;
    virtual ITreeNode *parent() const = 0;
};

#endif // ITREENODE_H
