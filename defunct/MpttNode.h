/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MPTTNODE_H
#define MPTTNODE_H

#include <QtCore/QList>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Forward declarations
struct MpttNode;
class TreeNode;

//! Global function for completely freeing all memory used by each MpttNode including any allocated TreeNodes and clears mpttNodeList
void freeMpttList(QList<MpttNode *> &mpttNodeList);

/**
  * Modified Preorder Tree Traversal representation of a TreeNode
  *
  * MPTT is an alternative tree form comprised of MPTT nodes, which simply consist of a TreeNode and a pair of integers,
  * left and right. Within the context of all other nodes, left and right denote a given node position within the tree.
  * MPTT is most useful for reading and writing trees to a database without resorting to recursion.
  *
  * This class is meant to serve as a carrier for the TreeNode pointer and it is the users responsibility to make sure
  * that the pointer is properly nulled and/or deallocated.
  *
  * Ultimately, should probably use implicit sharing for the TreeNode class...
  */
struct MpttNode
{
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Constructs an MpttNode that points to treeNode (default 0) and has the left and right MPTT values (default -1)
    MpttNode(TreeNode *treeNode = 0, int left = -1, int right = -1);

    // ------------------------------------------------------------------------------------------------
    // Public members
    TreeNode *treeNode_;        //!< Pointer to the corresponding TreeNode
    int left_;                  //!< Left MPTT value
    int right_;                 //!< Right MPTT value
};

// Tell Qt how to handle this data structure so it can optimize it's container usage and copying performance
Q_DECLARE_TYPEINFO(MpttNode, Q_MOVABLE_TYPE);


// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param treeNode [TreeNode *]
  * @param left [int]
  * @param right [int]
  */
inline
MpttNode::MpttNode(TreeNode *treeNode, int left, int right) : treeNode_(treeNode), left_(left), right_(right)
{
}

#endif // MPTTNODE_H
