/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MPTTNODE_H
#define MPTTNODE_H

#include "TreeNode.h"
#include "global.h"

// It is advisable to tell Qt how to handle this data structure so it can optimize it's container usage and copying
// performance. Use Q_DECLARE_TYPEINFO(...) for each user type
/**
  * Modified Preorder Tree Traversal representation of a TreeNode
  *
  * MPTT is an alternative tree form comprised of MPTT nodes, which simply consist of a TreeNode and a pair of integers,
  * left and right. Within the context of all other nodes, left and right denote a given node position within the tree.
  * MPTT is most useful for reading and writing trees to a database without resorting to recursion.
  *
  * This class is meant to serve as a carrier for the TreeNode pointer and it is the users responsibility to make sure
  * that the pointer is properly nulled and/or deallocated.
  */
template<typename Derived>
struct MpttNode
{
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Constructs an MpttNode that points to treeNode (default nullptr) and has the left and right MPTT values (default -1)
    MpttNode(Derived *treeNode = nullptr, int left = -1, int right = -1);

    bool operator==(const MpttNode &other) const
    {
        return treeNode_ == other.treeNode_ &&
               left_ == other.left_ &&
               right_ == other.right_;
    }
    bool operator!=(const MpttNode &other) const
    {
        return !operator==(other);
    }

    // ------------------------------------------------------------------------------------------------
    // Public members
    Derived *treeNode_;              //!< Pointer to the corresponding TreeNode
    int left_;                       //!< Left MPTT value
    int right_;                      //!< Right MPTT value
};


// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param treeNode [TreeNode *]
  * @param left [int]
  * @param right [int]
  */
template<typename Derived>
MpttNode<Derived>::MpttNode(Derived *treeNode, int left, int right)
    : treeNode_(treeNode),
      left_(left),
      right_(right)
{
}

#endif // MPTTNODE_H
