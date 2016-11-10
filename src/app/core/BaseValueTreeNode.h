/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BASEVALUETREENODE_H
#define BASEVALUETREENODE_H

#include "TreeNode.h"

/**
  * BaseValueTreeNode extends the basic TreeNode with a data member stored by value.
  */
template<typename T, typename D>
class BaseValueTreeNode : public TreeNode<D>
{
public:
    BaseValueTreeNode();
    explicit BaseValueTreeNode(T &data);            //!< Constructs an empty node whose data member is initialized to data
    explicit BaseValueTreeNode(const T &data);      //!< Constructs an empty node whose data member is initialized to data

    T data_;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  */
template<typename T, typename D>
inline
BaseValueTreeNode<T, D>::BaseValueTreeNode()
{
}

/**
  * @param data [T &]
  */
template<typename T, typename D>
inline
BaseValueTreeNode<T, D>::BaseValueTreeNode(T &data)
    : data_(data)
{
}

/**
  * @param data [T &]
  */
template<typename T, typename D>
inline
BaseValueTreeNode<T, D>::BaseValueTreeNode(const T &data)
    : data_(data)
{
}

#endif // BASEVALUETREENODE_H
