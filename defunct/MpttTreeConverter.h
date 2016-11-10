/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MPTTTREECONVERTER_H
#define MPTTTREECONVERTER_H

#include <QtCore/QList>

// ------------------------------------------------------------------------------------------------
// Forward declarations
struct MpttNode;
class TreeNode;

/**
  * MpttTreeConverter provides a purely static interface for converting between an in-memory TreeNode
  * tree representation and its MPTT-encoded equivalent representation.
  *
  * These methods do not require any private data, so they are all defined statically.
  */
class MpttTreeConverter
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public static methods
    //! Returns an equivalent representation of treeNode and its descendants in the form of a list of Mptt nodes
    static QList<MpttNode *> toMpttList(TreeNode *treeNode);

    //! Static function for transforming a set of MpttNodes into a tree structure. Returns the root node on success or 0 otherwise.
    static TreeNode *fromMpttList(QList<MpttNode *> &mpttNodeList);

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! Recursive function that does pre-order tree traversal beginning with node and adding adding new nodes mpttNodeList. Returns the rightmost value of the last MPTT node.
    static int recurseBuildMpttList(TreeNode *node, int left, QList<MpttNode *> *mpttNodeList);

    //! Internal function for use with sorting a QList<MpttNode *> by the left values in ascending order
    static bool sortMpttLeftLessThan(const MpttNode *a, const MpttNode *b);

    //!< Internal function for validating a MPTT tree
    static void validateMpttList(const QList<MpttNode *> &mpttNodeList);
};

#endif // MPTTTREECONVERTER_H
