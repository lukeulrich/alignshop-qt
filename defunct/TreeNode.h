/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef TREENODE_H
#define TREENODE_H

#include <QtCore/QList>

#include "MpttNode.h"

/**
  * A generic tree node
  *
  * A basic tree node contains a pointer to its parent and a list of pointers to any children. A root node does not
  * have any parent (parent_ = 0). This class merely maintains the information necesary to maintain a list of child
  * nodes and its parent. Thus, no other data members are present.
  *
  * A TreeNode takes ownership of all its children nodes and handles deallocating them. Thus, deleting a parent node
  * will also free all descendants. The same applies to removal functions (e.g. removeChildAt, removeChildren, etc).
  *
  * This class is intended to be sublcassed and enhanced with application specific data members and methods.
  */
class TreeNode
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    TreeNode();                                                     //!< Constructs an empty node, which by definition is a root node
    explicit TreeNode(const TreeNode &other);                       //!< Empty copy constructor declared to enable copying in subclasses; initializes parent_ to zero

    // ------------------------------------------------------------------------------------------------
    // Destructors
    virtual ~TreeNode();                                            //!< TreeNode destructor

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void appendChild(TreeNode *treeNode);                           //!< Appends treeNode to this node's list of children
    void appendChildren(const QList<TreeNode *> &treeNodes);        //!< Appends treeNodes to this node's list of children
    TreeNode *childAt(int row) const;                               //!< Return the TreeNode child at position row
    QList<TreeNode *> childrenBetween(int start, int end) const;    //!< Return a list of children TreeNodes between start and end inclusive
    int childCount() const;                                         //!< Return this node's number of children
    void insertChildAt(int row, TreeNode *treeNode);                //!< Insert treeNode at position row
    bool isDescendantOf(const TreeNode *otherTreeNode) const;       //!< Returns true if this TreeNode is a descendant of otherTreeNode; false otherwise
    TreeNode *parent() const;                                       //!< Return the parent TreeNode
    int row() const;                                                //!< Return the current position of this TreeNode relative to the parent node
    void removeChildAt(int row);                                    //!< Remove the child node at row (freeing it and all descendants) and return whether this operation was successful
    void removeChildren();                                          //!< Remove all child nodes
    TreeNode *takeChildAt(int row);                                 //!< Removes the child node at row row and returns a pointer to this node on success or 0 otherwise
    QList<TreeNode *> takeChildren();                               //!< Removes all child nodes and returns a list of pointers to all nodes that were removed

    friend class MpttTreeConverter;
protected:
    // ------------------------------------------------------------------------------------------------
    // Protected members
    TreeNode *parent_;                  //!< Pointer to the parent TreeNode
    QList<TreeNode *> children_;        //!< List of children TreeNode

private:
    // Prevent the default assignment operator
    TreeNode &operator=(const TreeNode &other);
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  */
inline
TreeNode::TreeNode() : parent_(0)
{
}

/**
  * @param other [const TreeNode &]
  */
inline
TreeNode::TreeNode(const TreeNode &other) : parent_(0)
{
    Q_UNUSED(other);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
/**
  * Free all memory held by child nodes and remove ourselves from any parent list of children.
  */
inline
TreeNode::~TreeNode()
{
    if (parent_)
    {
        Q_ASSERT_X(row() != -1, "TreeNode::~TreeNode", "Valid parent node, but child node not present in parent list");

        // We are a non-root node, thus it is important to remove ourselves from the parent node.
        // Otherwise, a double-free crash will occur when the parent node is freed
        parent_->children_.removeAt(row());
    }

    qDeleteAll(children_);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param treeNode [TreeNode *]
  */
inline
void TreeNode::appendChild(TreeNode *treeNode)
{
    Q_ASSERT_X(treeNode, "TreeNode::appendChild", "treeNode must not be null");
    Q_ASSERT_X(treeNode != this, "TreeNode::appendChild", "treeNode must not be this");

    if (treeNode && treeNode != this)   // Release mode guard
    {
        children_.append(treeNode);
        treeNode->parent_ = this;
    }
}

/**
  * Convenience function that calls appendChild for every TreeNode * in treeNodes.
  *
  * @param treeNodes [const QList<TreeNode *> &]
  */
inline
void TreeNode::appendChildren(const QList<TreeNode *> &treeNodes)
{
    for (int i=0, z=treeNodes.count(); i<z; ++i)
        appendChild(treeNodes.at(i));
}

/**
  * @param row [int]
  * @return TreeNode *
  */
inline
TreeNode *TreeNode::childAt(int row) const
{
    Q_ASSERT_X(row >= 0, "TreeNode::child", "row out of range; must be >= 0");
    Q_ASSERT_X(row < children_.count(), "TreeNode::child", "row out of range; must be < number of child nodes");

    return children_.at(row);
}

/**
  * Returns the current row of this child within its parent children list. Root nodes are a special case in which they
  * are the only row and conceptually represent the first and only row. Thus, for root nodes, this returns 0.
  *
  * @return int
  */
inline
int TreeNode::row() const
{
    if (parent_)
        return parent_->children_.indexOf(const_cast<TreeNode *>(this));

    return 0;
}

/**
  * @param start [int]
  * @param end [int]
  * @returns QList<TreeNode *>
  */
inline
QList<TreeNode *> TreeNode::childrenBetween(int start, int end) const
{
    Q_ASSERT_X(children_.count(), "TreeNode::childrenBetween", "Must have at least one child");
    Q_ASSERT_X(start >= 0, "TreeNode::childrenBetween", "start must be >= 0");
    Q_ASSERT_X(end < children_.count(), "TreeNode::childrenBetween", "end must be less than the number of children");
    Q_ASSERT_X(start <= end, "TreeNode::childrenBetween", "start index out of range; must be <= end");

    return children_.mid(start, end-start+1);
}

/**
  * @return int
  */
inline
int TreeNode::childCount() const
{
    return children_.count();
}

/**
  * @param row [int]
  * @param treeNode [TreeNode *]
  */
inline
void TreeNode::insertChildAt(int row, TreeNode *treeNode)
{
    Q_ASSERT_X(row >= 0, "TreeNode::insertChildAt", "row out of range, may not be negative");
    Q_ASSERT_X(row <= childCount(), "TreeNode::insertChildAt", "row out of range, must be at least childCount()");

    Q_ASSERT_X(treeNode, "TreeNode::insertChildAt", "treeNode must not be null");
    Q_ASSERT_X(treeNode != this, "TreeNode::insertChildAt", "treeNode must not be this");

    if (treeNode != this)
    {
        children_.insert(row, treeNode);
        treeNode->parent_ = this;
    }
}

/**
  * Simply walks up the list of parents checking to see if any are equivalent to otherTreeNode.
  *
  * @param otherTreeNode [const TreeNode *]
  * @returns bool
  */
inline
bool TreeNode::isDescendantOf(const TreeNode *otherTreeNode) const
{
    const TreeNode *tmp = parent_;
    while (tmp)
    {
        if (tmp == otherTreeNode)
            return true;

        tmp = tmp->parent_;
    }

    return false;
}


/**
  * @return TreeNode *
  */
inline
TreeNode *TreeNode::parent() const
{
    return parent_;
}

/**
  * @param int
  */
inline
void TreeNode::removeChildAt(int row)
{
    Q_ASSERT_X(row >= 0, "TreeNode::removeChildAt", "row out of range; must be >= 0");
    Q_ASSERT_X(row <= children_.count(), "TreeNode::removeChildAt", "row out of range; must be <= number of child nodes");

    // There are multiple methods for retrieving the TreeNode pointer inside the children_ list. Since we remove the item
    // from its parent list when it is deleted, we do not remove from the list here, but rather, simply delete it and
    // defer list bookkeeping to the destructor.
    //
    // On second thought, we'll go ahead and do a slight optimization here. We take the TreeNode from the list and set
    // its parent_ to 0 so that it won't try to remove it from its parent list upon destruction. This saves us an
    // indexOf function call in the destructor.
    TreeNode *treeNode = children_.takeAt(row);
    treeNode->parent_ = 0;
    delete treeNode;
    treeNode = 0;
}


/**
  */
inline
void TreeNode::removeChildren()
{
    for (int i=children_.count()-1; i>= 0; --i)
        removeChildAt(i);
}

/**
  * [UNTESTED]
  * @param row [int]
  * @returns TreeNode *
  */
inline
TreeNode *TreeNode::takeChildAt(int row)
{
    Q_ASSERT_X(row >= 0, "TreeNode::takeChildAt", "row out of range; must be >= 0");
    Q_ASSERT_X(row <= children_.count(), "TreeNode::takeChildAt", "row out of range; must be <= number of child nodes");

    // If you take a child, it's parent is set to 0 and it effectively becomes a root node
    TreeNode *node = children_.takeAt(row);
    node->parent_ = 0;
    return node;
}

/**
  * Removes all children from this node without deallocating them, sets their parent to zero (effectively
  * making each one a root node), and returns this list.
  *
  * @returns QList<TreeNode *>
  * @see takeChildAt()
  */
inline
QList<TreeNode *> TreeNode::takeChildren()
{
    QList<TreeNode *> children = children_;
    foreach (TreeNode *child, children)
        child->parent_ = 0;

    children_.clear();

    return children;
}

#endif // TREENODE_H
