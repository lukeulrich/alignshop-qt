/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BASEPOINTERTREENODE_H
#define BASEPOINTERTREENODE_H

#include "TreeNode.h"
#include "global.h"

/**
  * BasePointerTreeNode is a specialized type of TreeNode that only accepts pointers for its internal data structure and
  * deletes this memory upon destruction.
  *
  * The motivation for BasePointerTreeNode stemmed from the fact that external methods would be required for deallocating
  * any memory associated with each TreeNode. While technically possible to pass a pointer to the templated TreeNode
  * class, this would create a tight coupling between TreeNodes that contained pointer data (e.g. Tasks) and its many
  * methods that involve removing TreeNodes. Rather, it is desirable for any data pointer to be automatically
  * deallocated whenever its owning TreeNode is destroyed.
  *
  * Only accepts pointers for the public data member because the constructor requires a pointer template argument.
  */
template<typename T, typename D>
class BasePointerTreeNode : public TreeNode<D>
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit BasePointerTreeNode(T *data);              //!< Constructor that takes ownership of data
    ~BasePointerTreeNode();                             //!< Destroys all children and this node

    T *take()
    {
        T *tmp = data_;
        data_ = nullptr;
        return tmp;
    }

    T *data_;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param data [T *]
  */
template<typename T, typename D>
inline
BasePointerTreeNode<T, D>::BasePointerTreeNode(T *data)
    : data_(data)
{
}

/**
  * It is desirable to recursively free all heap allocated pointers beginning with the leaves and working backwards to
  * this node. This is not possible by merely freeing the data in this method; rather, it is necessary to handle
  * removing all the children from this method and not utilize the TreeNode destructors implementation. So we duplicate
  * it here.
  */
template<typename T, typename D>
inline
BasePointerTreeNode<T, D>::~BasePointerTreeNode()
{
    if (this->parent_ && !this->parent_->defunct_)
    {
        ASSERT_X(this->row() != -1, "Valid parent node, but child node not present in parent list");

        // We are a non-root node, thus it is important to remove ourselves from the parent node.
        // Otherwise, a double-free crash will occur when the parent node is freed
        this->parent_->children_.remove(this->row());
    }

    // This node will no longer be around, therefore mark it as defunct so that children will not have to remove
    // themselves from the parent node.
    this->defunct_ = true;

    qDeleteAll(this->children_);

    this->children_.clear();  // To save the base class constructor from repeating all this effort.

    delete data_;
}


#endif // BASEPOINTERTREENODE_H
