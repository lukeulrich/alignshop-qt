/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MPTT_TPP
#define MPTT_TPP

#include <QtCore/QtAlgorithms>
#include <QtCore/QSet>
#include <QtCore/QVector>

#include "exceptions/InvalidMpttNodeError.h"
#include "global.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Forward declarations for private methods
namespace
{
    //! Internal function for use with sorting a QVector<MpttNode> by the left values in ascending order
    template<typename Derived>
    bool sortMpttLeftLessThan(const MpttNode<Derived> &a, const MpttNode<Derived> &b);

    //! Recursive function that does pre-order tree traversal beginning with node and adding adding new nodes mpttNodeVector. Returns the rightmost value of the last MPTT node.
    template<typename Derived>
    int recurseBuildMpttVector(Derived *node, int left, QVector<MpttNode<Derived> > &mpttNodeVector);
//    int recurseBuildMpttVector(TreeNode<Derived> *node, int left, QVector<MpttNode<Derived> > &mpttNodeVector);

    //!< Internal function for validating a MPTT tree; throws an exception if an error is encountered
    template<typename Derived>
    void validateMpttVector(const QVector<MpttNode<Derived> > &mpttNodeVector);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Methods
/**
  * @param mpttNodeVector [const QVector<MpttNode<Derived> > &]
  */
template<typename Derived>
void Mptt::freeMpttTreeNodes(QVector<MpttNode<Derived> > &mpttNodeVector)
{
    for (int i=0, z=mpttNodeVector.size(); i<z; ++i)
//    QVector<MpttNode<Derived> >::Iterator it = mpttNodeVector.begin();
//    for (QVector<MpttNode<Derived> >::Iterator end = mpttNodeVector.end(); it != end; ++it)
    {
//        MpttNode<Derived> &mpttNode = *it;
        MpttNode<Derived> &mpttNode = mpttNodeVector[i];
        if (mpttNode.treeNode_ != nullptr)
        {
            delete mpttNode.treeNode_;
            mpttNode.treeNode_ = nullptr;
        }
    }
}

/**
  * Takes a given TreeNode and builds a corresponding list of MpttNodes that describe the tree structure.
  * Because the in-memory structure cannot be in an invalid state, there are no exceptions that may be
  * thrown via this method (unlike its sister method, fromMpttVector).
  *
  * @param treeNode [TreeNode<Derived> *]
  * @return QVector<MpttNode<Derived> >
  * @see fromMpttVector()
  */
template<typename Derived>
//QVector<MpttNode<Derived> > Mptt::toMpttVector(TreeNode<Derived> *treeNode)
QVector<MpttNode<Derived> > Mptt::toMpttVector(Derived *treeNode)
{
    QVector<MpttNode<Derived> > mpttNodes;

    if (treeNode != nullptr)
        recurseBuildMpttVector(treeNode, 0, mpttNodes);

    return mpttNodes;
}

/**
  * Analyzes the nodes in mpttNodeVector for validity and builds a tree from the given MPTT nodes. If an error occurs
  * during the construction process, this function throws an exception.
  *
  * All TreeNode pointers should have been previously initialized and allocated; however, no parent/child relationships
  * should be defined or an empty tree will be returned. This function merely rearranges them from an MPTT flat list
  * into its expected hierarchy. Consequently, this function will not work as expected if passed the result from a
  * toMpttVector() because presumably there will be parent/child relationships defined.
  *
  * Performs a consistency/validity check before processing the tree. This way the TreeNode pointers are not partially
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
  * If mpttNodeVector is empty, then simply return 0, but do not throw an error.
  *
  * The various errors that may occur are:
  * o mpttNodeVector is empty
  * o duplicate TreeNode pointer found
  * o root node left value does not equal 1
  * o root node right value does not equal mpttNodeVector.count() * 2
  * o found node with left == right
  * o left is not less than right
  * o left may not equal parent left
  * o left must be greater than parent left <-- Not sure if this can ever happen since we sort the list on the left
  *   ascending
  * o right may not equal parent right
  * o right must be less than parent right
  *
  * Sorts the MpttNode list referenced by mpttNodeVector if it is not already sorted.
  *
  * Utilizes all RAII during point at which Error may be thrown and therefore this method is exception safe.
  *
  * @param mpttNodeVector [const QVector<MpttNode<Derived> > &]
  * @return TreeNode<Derived> *
  * @see toMpttVector()
  *
  * @throws InvalidMpttNodeError
  *
  * TODO: Refactor validation of MPTT tree (encoded within mpttNodeVector) to external class and then call
  *       that method here. Currently, we have combined these two functions inside a single function.
  * Partial fix: moved validation code to private function, validateMpttVector
  *
  * FUTURE OPTIMIZATION: Provide option to skip the sort routine in the event that mpttNodeVector is already sorted.
  */
template<typename Derived>
Derived *Mptt::fromMpttVector(QVector<MpttNode<Derived> > &mpttNodeVector)
{
    if (mpttNodeVector.size() == 0)
        return nullptr;

    // ----------------------------------------------------------------------------------
    // Check the MPTT node list for consistency
    // Sort the nodes by their left value
    qSort(mpttNodeVector.begin(), mpttNodeVector.end(), sortMpttLeftLessThan<Derived>);

    validateMpttVector(mpttNodeVector); // <-- This method is totally responsible for throwing any InvalidMpttNodeError's

    // ----------------------------------------------------------------------------------
    // Should be safe to assume that the tree is error-free
    // Build tree for good at this point
    Derived *root = mpttNodeVector.first().treeNode_;

    // Walk down the mptt list and iteratively build the tree
    QVector<MpttNode<Derived> > parentMptts;
    parentMptts << mpttNodeVector.first();
    for (int i=1, z=mpttNodeVector.count(); i<z; ++i)
    {
        MpttNode<Derived> mpttNode = mpttNodeVector.at(i);

        while (parentMptts.last().right_ < mpttNode.right_)
            parentMptts.pop_back();

        parentMptts.last().treeNode_->appendChild(mpttNode.treeNode_);
        parentMptts << mpttNode;
    }

    return root;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private method definitions

namespace
{
    /**
      * Performs a pre-order tree traversal of all treenodes under node and adds a new MpttNode to mpttNodeVector for every
      * new row. mpttNodeVector is a pointer so that we avoid the overhead of QVector implicit sharing. Left denotes the left
      * MPTT value for node and is used to compute the remaining nodes right MPTT value.
      *
      * @param node [TreeNode *]
      * @param left [int]
      * @param mpttNodeVector [QVector<MpttNode *> *]
      */
    template<typename Derived>
    int recurseBuildMpttVector(Derived *node, int left, QVector<MpttNode<Derived> >         &mpttNodeVector)
     //  int recurseBuildMpttVector(TreeNode<Derived> *node, int left, QVector<MpttNode<Derived> > &mpttNodeVector)
    {
        ASSERT_X(node != nullptr, "node must not be null");
        ASSERT_X(left >= 0, "left must be a positive, non-zero value");

        mpttNodeVector << MpttNode<Derived>(static_cast<Derived *>(node), left + 1, left + 1);
        int index = mpttNodeVector.size() - 1;

        int right_value = left + 1;
        for (int i=0, z=node->childCount(); i<z; ++i)
            right_value = recurseBuildMpttVector(static_cast<Derived *>(node->childAt(i)), right_value, mpttNodeVector);
        ++right_value;

        mpttNodeVector[index].right_ = right_value;

        return right_value;
    }

    /**
      * Simply returns whether the left value of node a is less than the left value of node b
      *
      * @param a [const MpttNode<Derived> &]
      * @param b [const MpttNode<Derived> &]
      */
    template<typename Derived>
    bool sortMpttLeftLessThan(const MpttNode<Derived> &a, const MpttNode<Derived> &b)
    {
        return a.left_ < b.left_;
    }

    /**
      * Iterate through mpttNodeVector and throw an InvalidMpttNodeError upon finding an MPTT inconsistency.
      *
      * @param mpttNodeVector [const QVector<MpttNode<Derived> > &]
      *
      * @throws InvalidMpttNodeError
      */
    template<typename Derived>
    void validateMpttVector(const QVector<MpttNode<Derived> > &mpttNodeVector)
    {
        if (mpttNodeVector.isEmpty())
            return;

        const MpttNode<Derived> &rootMptt = mpttNodeVector.first();

        // Confirm that the root left value is 1
        if (rootMptt.left_ != 1)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eBadLeftCode, "Root node left value does not equal 1" /* , rootMptt */);

        // Confirm that the root right value is the list size * 2
        if (rootMptt.right_ != mpttNodeVector.count() * 2)
            throw InvalidMpttNodeError(InvalidMpttNodeError::eBadRightCode, "Root node right value does not equal mpttNodeVector.count() * 2" /* , rootMptt */);

        // Confirm that the root treeNode pointer is valid
        ASSERT_X(rootMptt.treeNode_, "Invalid root treeNode_ pointer associated with MpttNode");
        if (!rootMptt.treeNode_)   // Release code only test case
            throw InvalidMpttNodeError(InvalidMpttNodeError::eNullTreeNodeCode, "Root Mptt node contains invalid treeNode_ pointer" /* , rootMptt */);

        if (rootMptt.treeNode_->parent())
            throw InvalidMpttNodeError(InvalidMpttNodeError::eParentTreeNodeCode, "Root Mptt node contains parent pointer" /*, rootMptt */);

        if (rootMptt.treeNode_->childCount())
            throw InvalidMpttNodeError(InvalidMpttNodeError::eHasChildrenCode, "Root Mptt node is not allowed to have children" /*, rootMptt */);

        // Loop over every entry (except root which has already been checked) in the MPTT list and check each node for consistency
        QVector<MpttNode<Derived> > parentMptts;
        QSet<Derived *> seenTreeNode;
        QSet<int> seenLeftRight;

        parentMptts << rootMptt;
        seenTreeNode << rootMptt.treeNode_;
        seenLeftRight << rootMptt.left_;
        seenLeftRight << rootMptt.right_;

        int expectedLeft = 2;
        int expectedUpperRightBound = rootMptt.right_ - 1; // Inclusive
        for (int i=1, z=mpttNodeVector.size(); i<z; ++i)
        {
            // Confirm that all mptt nodes do not have pre-existing relationships defined and that there are no duplicate
            // TreeNode pointers
            const MpttNode<Derived> &mpttNode = mpttNodeVector.at(i);
            Derived *treeNode = mpttNode.treeNode_;
            ASSERT_X(treeNode != nullptr, "Invalid treeNode_ pointer associated with MpttNode");
            if (treeNode == nullptr)  // Test case for release code
                throw InvalidMpttNodeError(InvalidMpttNodeError::eNullTreeNodeCode, QString("Mptt node contains invalid treeNode_ pointer (Mptt node: %1)").arg(i) /*, mpttNode */);

            if (seenTreeNode.contains(treeNode))
                throw InvalidMpttNodeError(InvalidMpttNodeError::eDuplicateTreeNodeCode, "Duplicate TreeNode pointer found" /*, mpttNode */);

            if (treeNode->parent())
                throw InvalidMpttNodeError(InvalidMpttNodeError::eParentTreeNodeCode, QString("Mptt node is not allowed to have a defined parent (Mptt node: %1)").arg(i) /*, mpttNode */);

            if (treeNode->childCount())
                throw InvalidMpttNodeError(InvalidMpttNodeError::eHasChildrenCode, QString("Mptt node is not allowed to have children (Mptt node: %1)").arg(i) /*, mpttNode */);

            if (seenLeftRight.contains(mpttNode.left_))
                throw InvalidMpttNodeError(InvalidMpttNodeError::eDuplicateLeftCode, QString("Duplicate left value (%1, Mptt node: %2)").arg(mpttNode.left_, i) /*, mpttNode */);

            if (seenLeftRight.contains(mpttNode.right_))
                throw InvalidMpttNodeError(InvalidMpttNodeError::eDuplicateRightCode, QString("Duplicate right value (%1, Mptt node: %2").arg(mpttNode.right_, i) /*, mpttNode */);

            if (mpttNode.left_ >= mpttNode.right_)
                throw InvalidMpttNodeError(InvalidMpttNodeError::eBadLeftCode, QString("Left value (%1) must be less than right value (%2)").arg(mpttNode.left_, mpttNode.right_) /*, mpttNode */);

            if (mpttNode.left_ % 2 == mpttNode.right_ % 2)
                throw InvalidMpttNodeError(InvalidMpttNodeError::eEvenOddCode, QString("Left value (%1) and right value (%2) cannot both be even or both be odd").arg(mpttNode.left_, mpttNode.right_) /*, mpttNode */);

            if (mpttNode.left_ > parentMptts.last().left_ &&
                mpttNode.left_ < parentMptts.last().right_ &&
                mpttNode.right_ > parentMptts.last().right_)
            {
                // Safe to assume that the right should be within these bounds as well
                throw InvalidMpttNodeError(InvalidMpttNodeError::eUnexpectedRightCode, QString("Unexpected right value (%1)").arg(mpttNode.right_) /*, mpttNode */);
            }

            // Reset to the proper parent
            while (parentMptts.last().right_ < mpttNode.right_)
            {
                expectedLeft = parentMptts.last().right_ + 1;
                parentMptts.pop_back();
                expectedUpperRightBound = parentMptts.last().right_ - 1;
            }

            if (mpttNode.right_ > expectedUpperRightBound)
                throw InvalidMpttNodeError(InvalidMpttNodeError::eUnexpectedRightCode, QString("Unexpected right value (%1)").arg(mpttNode.right_) /*, mpttNode */);

            if (mpttNode.left_ != expectedLeft)
                throw InvalidMpttNodeError(InvalidMpttNodeError::eUnexpectedLeftCode, QString("Unexpected left value (%1)").arg(mpttNode.left_) /*, mpttNode */);

            // Cannot be equal because we will have already added the parent values to the seen hash. Thus we only
            // test if < or >
            if (mpttNode.left_ < parentMptts.last().left_)
                throw InvalidMpttNodeError(InvalidMpttNodeError::eBadLeftCode, QString("Left value (%1) must be greater than the parent left value (%2").arg(mpttNode.left_, parentMptts.last().right_) /*, mpttNode */);

            if (mpttNode.right_ > parentMptts.last().right_)
                throw InvalidMpttNodeError(InvalidMpttNodeError::eBadRightCode, QString("Right value (%1) must be less than the parent right value (%2)").arg(mpttNode.right_, parentMptts.last().right_) /*, mpttNode */);

            // Mark TreeNode pointer, left and right values as having been seen
            seenTreeNode << treeNode;
            seenLeftRight << mpttNode.left_;
            seenLeftRight << mpttNode.right_;

            // Update what we expect the next left to look like. This purely depends on whether the
            // node has children
            if (mpttNode.left_ + 1 == mpttNode.right_)
                expectedLeft = mpttNode.left_ + 2;
            else
            {
                ++expectedLeft;
                parentMptts << mpttNode;
                expectedUpperRightBound = mpttNode.right_ - 1;
            }
        }

        // Confirm that one of each id/number was found within the tree - not sure how this condition
        // would pass and the above ones would not
        if (seenLeftRight.count() / 2 != mpttNodeVector.count())
            throw InvalidMpttNodeError(InvalidMpttNodeError::eMissingValueCode, "Missing at least one left or right value");
    }
}

#endif // MPTT_TPP
