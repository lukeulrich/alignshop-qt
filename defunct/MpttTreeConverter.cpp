/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "MpttTreeConverter.h"

#include <QtCore/QHash>

#include "exceptions/InvalidMpttNodeError.h"
#include "MpttNode.h"
#include "TreeNode.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Takes a given TreeNode and builds a corresponding list of MpttNodes that describe the tree structure.
  * Because the in-memory structure cannot be in an invalid state, there are no exceptions that may be
  * thrown via this method (unlike its sister method, fromMpttList).
  *
  * @param treeNode [TreeNode *]
  * @return QList<MpttNode *>
  * @see fromMpttList()
  */
QList<MpttNode *> MpttTreeConverter::toMpttList(TreeNode *treeNode)
{
    QList<MpttNode *> list;

    if (treeNode)
        recurseBuildMpttList(treeNode, 0, &list);

    return list;
}

/**
  * Analyzes the nodes in mpttNodeList for validity and builds a tree from the given MPTT nodes. If an error occurs
  * during the construction process, this function returns false and sets the error message in error.
  *
  * All TreeNode pointers should have been previously initialized and allocated; however, no parent/child relationships
  * should be defined or an empty tree will be returned. This function merely rearranges them from an MPTT flat list
  * into its expected hierarchy. Consequently, this function will not work as expected if passed the result from a
  * toMpttList() because presumably there will be parent/child relationships defined.
  *
  * Perform a consistency/validity check before processing the tree. This way the TreeNode pointers are not partially
  * rearranged if an error is encountered in the middle of processing the tree. This does mean that we have to loop
  * over the list twice though.
  *
  * Other constraints:
  * o duplicate TreeNode pointers are not allowed; would lead to cyclic graph
  * o The root MPTT left must be 1 and its right value = count(nodes) * 2
  * o Left and right values:
  *   >> must be integral
  *   >> must not have any gaps, that is missing integers between 1 and max
  *   >> left < right
  *   >> if left is even, then right must be odd and vice versa
  *   >> left > parent left and right < parent right
  *
  * If mpttNodeList is empty, then simply return 0, but do not throw an error.
  *
  * The various errors that may occur are:
  * o mpttNodeList is empty
  * o duplicate TreeNode pointer found
  * o root node left value does not equal 1
  * o root node right value does not equal mpttNodeList.count() * 2
  * o found node with left == right
  * o left is not less than right
  * o left may not equal parent left
  * o left must be greater than parent left <-- Not sure if this can ever happen since we sort the list on the left
  *   ascending
  * o right may not equal parent right
  * o right must be less than parent right
  *
  * Sorts the MpttNode list referenced by mpttNodeList if it is not already sorted.
  *
  * Utilizes all RAII during point at which Error may be thrown and therefore this method is exception safe.
  *
  * @param mpttNodeList [QList<MpttNode *>]
  * @return TreeNode *
  * @see toMpttList()
  *
  * @throws InvalidMpttNodeError
  *
  * TODO: Refactor validation of MPTT tree (encoded within mpttNodeList) to external class and then call
  *       that method here. Currently, we have combined these two functions inside a single function.
  * Partial fix: moved validation code to private function, validateMpttList
  *
  * FUTURE OPTIMIZATION: Provide option to skip the sort routine in the event that mpttNodeList is already sorted.
  */
TreeNode *MpttTreeConverter::fromMpttList(QList<MpttNode *> &mpttNodeList)
{
    if (mpttNodeList.count() == 0)
        return 0;

    // ----------------------------------------------------------------------------------
    // Check the MPTT node list for consistency
    // Sort the nodes by their left value
    qSort(mpttNodeList.begin(), mpttNodeList.end(), sortMpttLeftLessThan);

    validateMpttList(mpttNodeList); // <-- This method is totally responsible for throwing any InvalidMpttNodeError's

    // ----------------------------------------------------------------------------------
    // Should be safe to assume that the tree is error-free
    // Build tree for good at this point
    TreeNode *root = mpttNodeList.first()->treeNode_;

    // Walk down the mptt list and iteratively build the tree
    QList<MpttNode *> parentMptts;
    parentMptts.append(mpttNodeList.first());
    for (int i=1, z=mpttNodeList.count(); i<z; ++i)
    {
        MpttNode *mpttNode = mpttNodeList.at(i);

        while (parentMptts.last()->right_ < mpttNode->right_)
            parentMptts.pop_back();

        mpttNode->treeNode_->parent_ = parentMptts.last()->treeNode_;
        mpttNode->treeNode_->parent_->children_.append(mpttNode->treeNode_);

        parentMptts.append(mpttNode);
    }

    return root;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Performs a pre-order tree traversal of all treenodes under node and adds a new MpttNode to mpttNodeList for every
  * new row. mpttNodeList is a pointer so that we avoid the overhead of QList implicit sharing. Left denotes the left
  * MPTT value for node and is used to compute the remaining nodes right MPTT value.
  *
  * @param node [TreeNode *]
  * @param left [int]
  * @param mpttNodeList [QList<MpttNode *> *]
  */
int MpttTreeConverter::recurseBuildMpttList(TreeNode *node, int left, QList<MpttNode *> *mpttNodeList)
{
    Q_ASSERT_X(node, "TreeNode::recurseBuildMpttList", "node must not be empty");
    Q_ASSERT_X(left >= 0, "TreeNode::recurseBuildMpttList", "left must be a positive, non-zero value");
    Q_ASSERT_X(mpttNodeList, "TreeNode::recurseBuildMpttList", "mpttNodeList must not be null");

    MpttNode *mpttNode = new MpttNode(node, left+1, left+1);
    mpttNodeList->append(mpttNode);

    for (int i=0, z=node->childCount(); i<z; ++i)
        mpttNode->right_ = recurseBuildMpttList(node->children_.at(i), mpttNode->right_, mpttNodeList);

    ++mpttNode->right_;

    return mpttNode->right_;
}

/**
  * Simply returns whether the left value of node a is less than the left value of node b
  *
  * @param a [const MpttNode *]
  * @param b [const MpttNode *]
  */
bool MpttTreeConverter::sortMpttLeftLessThan(const MpttNode *a, const MpttNode *b)
{
    Q_ASSERT_X(a, "TreeNode::sortLeftLessThan", "a must be a non-empty value");
    Q_ASSERT_X(b, "TreeNode::sortLeftLessThan", "b must be a non-empty value");

    return a->left_ < b->left_;
}

/**
  * Iterate through mpttNodeList and throw an InvalidMpttNodeError upon finding an MPTT inconsistency.
  *
  * @param mpttNodeList [const QList<MpttNode *> &]
  *
  * @throws InvalidMpttNodeError
  */
void MpttTreeConverter::validateMpttList(const QList<MpttNode *> &mpttNodeList)
{
    if (mpttNodeList.isEmpty())
        return;

    MpttNode *rootMptt = mpttNodeList.first();

    // Confirm that the root left value is 1
    if (rootMptt->left_ != 1)
        throw InvalidMpttNodeError(InvalidMpttNodeError::eBadLeftCode, "Root node left value does not equal 1", rootMptt);

    // Confirm that the root right value is the list size * 2
    if (rootMptt->right_ != mpttNodeList.count() * 2)
        throw InvalidMpttNodeError(InvalidMpttNodeError::eBadRightCode, "Root node right value does not equal mpttNodeList.count() * 2", rootMptt);

    // Confirm that the root treeNode pointer is valid
    Q_ASSERT_X(rootMptt->treeNode_, "TreeNode::buildFromMpttList", "Invalid root treeNode_ pointer associated with MpttNode");
    if (!rootMptt->treeNode_)   // Release code only test case
        throw InvalidMpttNodeError(InvalidMpttNodeError::eNullTreeNodeCode, "Root Mptt node contains invalid treeNode_ pointer", rootMptt);

    if (rootMptt->treeNode_->parent_)
        throw InvalidMpttNodeError(InvalidMpttNodeError::eParentTreeNodeCode, "Root Mptt node contains parent pointer", rootMptt);

    if (rootMptt->treeNode_->childCount())
        throw InvalidMpttNodeError(InvalidMpttNodeError::eHasChildrenCode, "Root Mptt node is not allowed to have children", rootMptt);

    // Loop over every entry (except root which has already been checked) in the MPTT list and check each node for consistency
    QList<MpttNode *> parentMptts;
    QHash<TreeNode *, bool> seenTreeNode;
    QHash<int, bool> seenLeftRight;
    parentMptts.append(rootMptt);
    seenTreeNode[rootMptt->treeNode_] = true;
    seenLeftRight[rootMptt->left_] = true;
    seenLeftRight[rootMptt->right_] = true;
    int expectedLeft = 2;
    int expectedUpperRightBound = rootMptt->right_ - 1; // Inclusive
    for (int i=1, z=mpttNodeList.count(); i<z; ++i)
    {
        // Confirm that all mptt nodes do not have pre-existing relationships defined and that there are no duplicate
        // TreeNode pointers
        MpttNode *mpttNode = mpttNodeList.at(i);
        TreeNode *treeNode = mpttNode->treeNode_;
        Q_ASSERT_X(treeNode, "TreeNode::buildFromMpttList", "Invalid treeNode_ pointer associated with MpttNode");
        if (!treeNode)  // Test case for release code
            throw InvalidMpttNodeError(InvalidMpttNodeError::eNullTreeNodeCode, QString("Mptt node contains invalid treeNode_ pointer (Mptt node: %1)").arg(i), mpttNode);

        if (seenTreeNode.contains(treeNode))
            throw InvalidMpttNodeError(InvalidMpttNodeError::eDuplicateTreeNodeCode, "Duplicate TreeNode pointer found", mpttNode);

        if (treeNode->parent_)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eParentTreeNodeCode, QString("Mptt node is not allowed to have a defined parent (Mptt node: %1)").arg(i), mpttNode);

        if (treeNode->childCount())
            throw InvalidMpttNodeError(InvalidMpttNodeError::eHasChildrenCode, QString("Mptt node is not allowed to have children (Mptt node: %1)").arg(i), mpttNode);

        if (seenLeftRight.contains(mpttNode->left_))
            throw InvalidMpttNodeError(InvalidMpttNodeError::eDuplicateLeftCode, QString("Duplicate left value (%1, Mptt node: %2)").arg(mpttNode->left_, i), mpttNode);

        if (seenLeftRight.contains(mpttNode->right_))
            throw InvalidMpttNodeError(InvalidMpttNodeError::eDuplicateRightCode, QString("Duplicate right value (%1, Mptt node: %2").arg(mpttNode->right_, i), mpttNode);

        if (mpttNode->left_ >= mpttNode->right_)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eBadLeftCode, QString("Left value (%1) must be less than right value (%2)").arg(mpttNode->left_, mpttNode->right_), mpttNode);

        if (mpttNode->left_ % 2 == mpttNode->right_ % 2)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eEvenOddCode, QString("Left value (%1) and right value (%2) cannot both be even or both be odd").arg(mpttNode->left_, mpttNode->right_), mpttNode);

        if (mpttNode->left_ > parentMptts.last()->left_ &&
            mpttNode->left_ < parentMptts.last()->right_ &&
            mpttNode->right_ > parentMptts.last()->right_)
        {
            // Safe to assume that the right should be within these bounds as well
            throw InvalidMpttNodeError(InvalidMpttNodeError::eUnexpectedRightCode, QString("Unexpected right value (%1)").arg(mpttNode->right_), mpttNode);
        }

        // Reset to the proper parent
        while (parentMptts.last()->right_ < mpttNode->right_)
        {
            expectedLeft = parentMptts.last()->right_ + 1;
            parentMptts.pop_back();
            expectedUpperRightBound = parentMptts.last()->right_ - 1;
        }

        if (mpttNode->right_ > expectedUpperRightBound)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eUnexpectedRightCode, QString("Unexpected right value (%1)").arg(mpttNode->right_), mpttNode);

        if (mpttNode->left_ != expectedLeft)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eUnexpectedLeftCode, QString("Unexpected left value (%1)").arg(mpttNode->left_), mpttNode);

        // Cannot be equal because we will have already added the parent values to the seen hash. Thus we only
        // test if < or >
        if (mpttNode->left_ < parentMptts.last()->left_)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eBadLeftCode, QString("Left value (%1) must be greater than the parent left value (%2").arg(mpttNode->left_, parentMptts.last()->right_), mpttNode);

        if (mpttNode->right_ > parentMptts.last()->right_)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eBadRightCode, QString("Right value (%1) must be less than the parent right value (%2)").arg(mpttNode->right_, parentMptts.last()->right_), mpttNode);

        // Mark TreeNode pointer, left and right values as having been seen
        seenTreeNode[treeNode] = true;
        seenLeftRight[mpttNode->left_] = true;
        seenLeftRight[mpttNode->right_] = true;

        // Update what we expect the next left to look like. This purely depends on whether the
        // node has children
        if (mpttNode->left_ + 1 == mpttNode->right_)
            expectedLeft = mpttNode->left_ + 2;
        else
        {
            ++expectedLeft;
            parentMptts.append(mpttNode);
            expectedUpperRightBound = mpttNode->right_ - 1;
        }
    }

    // Confirm that one of each id/number was found within the tree - not sure how this condition
    // would pass and the above ones would not
    if (seenLeftRight.count() / 2 != mpttNodeList.count())
        throw InvalidMpttNodeError(InvalidMpttNodeError::eMissingValueCode, "Missing at least one left or right value");
}
