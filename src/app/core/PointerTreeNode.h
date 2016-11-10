/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef POINTERTREENODE_H
#define POINTERTREENODE_H

#include "BasePointerTreeNode.h"

/**
  * PointerTreeNode is a specialized type of TreeNode that only accepts pointers for its internal data structure and
  * deletes this memory upon destruction.
  *
  * The motivation for PointerTreeNode stemmed from the fact that external methods would be required for deallocating
  * any memory associated with each TreeNode. While technically possible to pass a pointer to the templated TreeNode
  * class, this would create a tight coupling between TreeNodes that contained pointer data (e.g. Tasks) and its many
  * methods that involve removing TreeNodes. Rather, it is desirable for any data pointer to be automatically
  * deallocated whenever its owning TreeNode is destroyed.
  *
  * Only accepts pointers for the public data member because the constructor requires a pointer template argument.
  */
template<typename T>
class PointerTreeNode : public BasePointerTreeNode<T, PointerTreeNode<T> >
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit PointerTreeNode(T *data);              //!< Constructor that takes ownership of data
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param data [T *]
  */
template<typename T>
inline
PointerTreeNode<T>::PointerTreeNode(T *data)
    : BasePointerTreeNode<T, PointerTreeNode<T> >(data)
{
}

#endif // TREENODE_H
