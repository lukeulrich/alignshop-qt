/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef VALUETREENODE_H
#define VALUETREENODE_H

#include "BaseValueTreeNode.h"

/**
  * ValueTreeNode extends the basic TreeNode with a data member stored by value.
  */
template<typename T>
class ValueTreeNode : public BaseValueTreeNode<T, ValueTreeNode<T> >
{
public:
    ValueTreeNode();
    explicit ValueTreeNode(T &data);            //!< Constructs an empty node whose data member is initialized to data
    explicit ValueTreeNode(const T &data);      //!< Constructs an empty node whose data member is initialized to data
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  */
template<typename T>
inline
ValueTreeNode<T>::ValueTreeNode()
{
}

/**
  * @param data [T &]
  */
template<typename T>
inline
ValueTreeNode<T>::ValueTreeNode(T &data)
    : BaseValueTreeNode<T, ValueTreeNode<T> >(data)
{
}

/**
  * @param data [T &]
  */
template<typename T>
inline
ValueTreeNode<T>::ValueTreeNode(const T &data)
    : BaseValueTreeNode<T, ValueTreeNode<T> >(data)
{
}

#endif // VALUETREENODE_H
