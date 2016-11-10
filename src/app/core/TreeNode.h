/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef TREENODE_H
#define TREENODE_H

#include <QtCore/QVector>

#include "macros.h"
#include "global.h"

/**
  * A generic tree node that merely provides core routines for managing tree structures.
  *
  * A basic tree node contains a pointer to its parent and a list of pointers to any children. A root node does not have
  * any parent (parent_ = nullptr). This class merely maintains the information necesary to maintain a list of child
  * nodes and its parent. Thus, no other data members are present.
  *
  * A TreeNode takes ownership of all its children nodes and handles deallocating them. Thus, deleting a parent node
  * will also free all its descendants. The same applies to removal functions (e.g. removeChildAt, removeChildren, etc).
  *
  * This class is intended to be subclassed and enhanced with application specific data members and methods.
  * Specifically, derived classes "inject" their type into this base class so that all tree methods return the relevant
  * subtype. This pattern is known as the Curiously Recurring Template Pattern.
  */
template<class Derived>
class TreeNode
{
public:
    typedef int IsDerivedFromTreeNode;

    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    TreeNode();                                                     //!< Constructs an empty node with a default constructed data member
    virtual ~TreeNode();                                            //!< TreeNode destructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void appendChild(Derived *treeNode);                            //!< Appends treeNode to this node's list of children
    void appendChildren(const QVector<Derived *> &treeNodes);       //!< Appends treeNodes to this node's list of children
    Derived *childAt(int row) const;                                //!< Return the TreeNode child at position row
    QVector<Derived *> children() const;                            //!< Returns a vector of all children
    QVector<Derived *> childrenBetween(int start, int end) const;   //!< Return a list of children TreeNodes between start and end inclusive
    int childCount() const;                                         //!< Return this node's number of children
    bool hasChildren() const;                                       //!< Returns true if there is at least one child node; false otherwise
    void insertChildAt(int row, Derived *treeNode);                 //!< Insert treeNode at position row
    bool isDescendantOf(const Derived *otherTreeNode) const;        //!< Returns true if this TreeNode is a descendant of otherTreeNode; false otherwise
    //! Returns true if this TreeNode is equal to or a descendant of any of the otherParent children between row and row + count - 1
    bool isEqualOrDescendantOfAny(const Derived *otherParent, int row, int count) const;
    bool isLeaf() const;                                            //!< Returns true if this node is a leaf node (does not have any children)
    bool isRoot() const;                                            //!< Returns true if this TreeNode does not have a parent
    Derived *nextAscendant() const;                                 //!< Returns the next non-child (ascendant, opposite of descendant) that would be returned in a pre-order fashion
    Derived *nextSibling() const;                                   //!< Returns the next sibling node or a null pointer if there is no additional sibling
    Derived *parent() const;                                        //!< Return the parent TreeNode
    Derived *root() const;                                          //!< Returns the root ancestor node
    int row() const;                                                //!< Return the current position of this TreeNode relative to the parent node
    void removeChildAt(int row);                                    //!< Remove the child node at row (freeing it and all descendants) and return whether this operation was successful
    void removeChildren();                                          //!< Remove all child nodes (freeing all associated memory)
    void removeChildren(int row, int count);                        //!< Remove count child nodes beginning at row (freeing all associated memory)
    Derived *takeChildAt(int row);                                  //!< Removes the child node at row row and returns a pointer to this node on success or 0 otherwise
    QVector<Derived *> takeChildren();                              //!< Removes all child nodes and returns a vector of pointers to all nodes that were removed
    QVector<Derived *> takeChildren(int row, int count);            //!< Removes count child nodes beginning at row and returns a vector of pointers to all nodes that were removed


    // ------------------------------------------------------------------------------------------------
    // Iterators
    /**
      * Generic pre-order traversal iterator
      */
    class iterator
    {
    public:
        inline iterator() : node_(nullptr) {}
        inline iterator(Derived *node) : node_(node) {}
        inline iterator(const iterator &other) : node_(other.node_) {}
//        inline Derived &operator*() { return node_->data_; }
        inline bool operator!=(const Derived *node) const { return node_ != node; }
        inline bool operator!=(const iterator &other) const { return node_ != other.node_; }
        inline bool operator==(Derived *node) const { return node_ == node; }
        inline bool operator==(const iterator &other) const { return node_ == other.node_; }
        inline bool operator()() const { return node_ != nullptr; }
        inline Derived *operator->() const { return node_; }
        inline iterator &operator++()
        {
            ASSERT(node_ != nullptr);
            node_ = (!node_->isLeaf()) ? node_->childAt(0) : node_->nextAscendant();
            return *this;
        }
        inline iterator operator++(int) { Derived *node = node_; operator++(); return node; }
        inline Derived *node() const { return node_; }

    private:
        Derived *node_;
    };
    friend class iterator;
    typedef iterator Iterator;

    /**
      * Generic pre-order traversal iterator
      */
    class const_iterator
    {
    public:
        inline const_iterator() : node_(nullptr) {}
        inline const_iterator(Derived *node) : node_(node) {}
        inline const_iterator(const Derived *node) : node_(node) {}
        inline const_iterator(const const_iterator &other) : node_(other.node_) {}
//        inline const Derived &operator*() const { return node_->data_; }
        inline bool operator!=(const Derived *node) const { return node_ != node; }
        inline bool operator!=(const const_iterator &other) const { return node_ != other.node_; }
        inline bool operator==(Derived *node) const { return node_ == node; }
        inline bool operator==(const const_iterator &other) const { return node_ == other.node_; }
        inline bool operator()() { return node_ != nullptr; }
        inline const Derived *operator->() const { return node_; }
        inline const_iterator &operator++()
        {
            ASSERT(node_ != nullptr);
            node_ = (!node_->isLeaf()) ? node_->childAt(0) : node_->nextAscendant();
            return *this;
        }
        inline const_iterator operator++(int) { const Derived *node = node_; operator++(); return node; }
        inline const Derived *node() const { return node_; }

    private:
        const Derived *node_;
    };
    friend class const_iterator;
    typedef const_iterator ConstIterator;


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected members
    Derived *parent_;                       //!< Pointer to the parent TreeNode
    QVector<Derived *> children_;           //!< List of children TreeNode
    bool defunct_;                           //!< Flag denoting whether this node is no longer valid (i.e. its destructor has been called)


private:
    // ------------------------------------------------------------------------------------------------
    // Private members

    friend class MpttTreeConverter;
    Q_DISABLE_COPY(TreeNode);                //!< Prevent all copying and assignment
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  */
template<typename Derived>
TreeNode<Derived>::TreeNode() : parent_(nullptr), defunct_(false)
{
}

/**
  * Free all memory held by child nodes and remove ourselves from any parent list of children. The defunct_ private
  * member serves as an optimization when destroying TreeNodes. If a parent is in the process of being destroyed, it is
  * unnecessary for the child nodes to remove themselves from the parent node. This state is captured using the defunct_
  * flag.
  */
template<typename Derived>
TreeNode<Derived>::~TreeNode()
{
    if (parent_ && !this->parent_->defunct_)
    {
        ASSERT_X(row() != -1, "Valid parent node, but child node not present in parent list");

        // We are a non-root node, thus it is important to remove ourselves from the parent node.
        // Otherwise, a double-free crash will occur when the parent node is freed
        parent_->children_.remove(row());
    }

    // This node will no longer be around, therefore mark it as defunct so that children will not have to remove
    // themselves from the parent node.
    defunct_ = true;

    qDeleteAll(children_);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param treeNode [Derived *]
  */
template<typename Derived>
void TreeNode<Derived>::appendChild(Derived *treeNode)
{
    Q_ASSERT_X(treeNode, "TreeNode<Derived>::appendChild", "treeNode must not be null");
    Q_ASSERT_X(treeNode != this, "TreeNode<Derived>::appendChild", "treeNode must not be this");
    Q_ASSERT_X(treeNode->parent_ == nullptr, "TreeNode<Derived>::appendChild", "treeNode must have a null parent pointer");

    children_.append(treeNode);
    treeNode->parent_ = static_cast<Derived *>(this);
}

/**
  * Convenience function that calls appendChild for every Derived * in treeNodes.
  *
  * @param treeNodes [const QVector<Derived *> &]
  */
template<typename Derived>
void TreeNode<Derived>::appendChildren(const QVector<Derived *> &treeNodes)
{
    for (int i=0, z=treeNodes.count(); i<z; ++i)
        appendChild(treeNodes.at(i));
}

/**
  * @param row [int]
  * @return Derived *
  */
template<typename Derived>
Derived *TreeNode<Derived>::childAt(int row) const
{
    Q_ASSERT_X(row >= 0, "TreeNode<Derived>::child", "row out of range; must be >= 0");
    Q_ASSERT_X(row < children_.count(), "TreeNode<Derived>::child", "row out of range; must be < number of child nodes");

    return children_.at(row);
}

/**
  * Returns the current row of this child within its parent children list. Root nodes are a special case in which they
  * are the only row and conceptually represent the first and only row. Thus, for root nodes, this returns 0.
  *
  * @return int
  */
template<typename Derived>
int TreeNode<Derived>::row() const
{
    if (parent_)
        return parent_->children_.indexOf(const_cast<Derived *>(static_cast<const Derived *>(this)));

    return 0;
}

/**
  * UNTESTED!
  *
  * @returns QVector<Derived *>
  */
template<typename Derived>
QVector<Derived *> TreeNode<Derived>::children() const
{
    return children_;
}

/**
  * @param start [int]
  * @param end [int]
  * @returns QVector<Derived *>
  */
template<typename Derived>
QVector<Derived *> TreeNode<Derived>::childrenBetween(int start, int end) const
{
    Q_ASSERT_X(children_.count(), "TreeNode<Derived>::childrenBetween", "Must have at least one child");
    Q_ASSERT_X(start >= 0, "TreeNode<Derived>::childrenBetween", "start must be >= 0");
    Q_ASSERT_X(end < children_.count(), "TreeNode<Derived>::childrenBetween", "end must be less than the number of children");
    Q_ASSERT_X(start <= end, "TreeNode<Derived>::childrenBetween", "start index out of range; must be <= end");

    return children_.mid(start, end-start+1);
}

/**
  * @return int
  */
template<typename Derived>
int TreeNode<Derived>::childCount() const
{
    return children_.count();
}

/**
  * @returns bool
  */
template<typename Derived>
bool TreeNode<Derived>::hasChildren() const
{
    return !children_.isEmpty();
}

/**
  * @param row [int]
  * @param treeNode [Derived *]
  */
template<typename Derived>
void TreeNode<Derived>::insertChildAt(int row, Derived *treeNode)
{
    Q_ASSERT_X(row >= 0, "TreeNode<Derived>::insertChildAt", "row out of range, may not be negative");
    Q_ASSERT_X(row <= childCount(), "TreeNode<Derived>::insertChildAt", "row out of range, must be at least childCount()");

    Q_ASSERT_X(treeNode, "TreeNode<Derived>::insertChildAt", "treeNode must not be null");
    Q_ASSERT_X(treeNode != this, "TreeNode<Derived>::insertChildAt", "treeNode must not be this");

    children_.insert(row, treeNode);
    treeNode->parent_ = static_cast<Derived *>(this);
}

/**
  * Simply walks up the list of parents checking to see if any are equivalent to otherTreeNode.
  *
  * @param otherTreeNode [const Derived *]
  * @returns bool
  */
template<typename Derived>
bool TreeNode<Derived>::isDescendantOf(const Derived *otherTreeNode) const
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
  * Convenience method for checking if any children of otherParent between row for count are equivalent to this node.
  *
  * @param otherParent [const Derived *]
  * @param row [int]
  * @param count [int]
  * @returns bool
  */
template<typename Derived>
bool TreeNode<Derived>::isEqualOrDescendantOfAny(const Derived *otherParent, int row, int count) const
{
    if (otherParent == nullptr)
        return false;

    ASSERT(row >= 0 && row + count - 1 < otherParent->childCount());

    for (int i=row, j=0; j< count; ++i, ++j)
        if (this == otherParent->children_.at(i) || isDescendantOf(otherParent->children_.at(i)))
            return true;

    return false;
}

/**
  * @returns bool
  */
template<typename Derived>
bool TreeNode<Derived>::isLeaf() const
{
    return children_.empty();
}

/**
  * @returns bool
  */
template<typename Derived>
bool TreeNode<Derived>::isRoot() const
{
    return parent_ == nullptr;
}

/**
  * A sibling has the same parent but is one more row down.
  *
  * @returns TreeNode *
  */
template<typename Derived>
Derived *TreeNode<Derived>::nextAscendant() const
{
    if (isRoot())
        return nullptr;

    const Derived *node = static_cast<const Derived *>(this);
    Derived *sibling = node->nextSibling();
    while (sibling == nullptr)
    {
        node = node->parent_;
        if (node->isRoot())
            break;

        sibling = node->nextSibling();
    }
    return sibling;
}

/**
  * A sibling has the same parent but is one more row down.
  *
  * @returns TreeNode *
  */
template<typename Derived>
Derived *TreeNode<Derived>::nextSibling() const
{
    if (parent_ != nullptr)
    {
        int r = row();
        if (parent_->childCount() - 1 > r)
            return parent_->childAt(r + 1);
    }

    return nullptr;
}

/**
  * @return TreeNode *
  */
template<typename Derived>
Derived *TreeNode<Derived>::parent() const
{
    return parent_;
}

/**
  * @returns TreeNode *
  */
template<typename Derived>
Derived *TreeNode<Derived>::root() const
{
    const Derived *node = static_cast<const Derived *>(this);
    while (!node->isRoot())
        node = node->parent_;

    // ISSUE?
    return const_cast<Derived *>(node);
}

/**
  * @param int
  */
template<typename Derived>
void TreeNode<Derived>::removeChildAt(int row)
{
    Q_ASSERT_X(row >= 0, "TreeNode<Derived>::removeChildAt", "row out of range; must be >= 0");
    Q_ASSERT_X(row <= children_.count(), "TreeNode<Derived>::removeChildAt", "row out of range; must be <= number of child nodes");

    // There are multiple methods for retrieving the TreeNode pointer inside the children_ list. Since we remove the item
    // from its parent list when it is deleted, we do not remove from the list here, but rather, simply delete it and
    // defer list bookkeeping to the destructor.
    //
    // On second thought, we'll go ahead and do a slight optimization here. We remove the TreeNode from the list and set
    // its parent_ to nullptr so that it won't try to remove it from its parent list upon destruction. This saves us an
    // indexOf function call in the destructor.
    Derived *child = children_.at(row);
    child->parent_ = nullptr;
    children_.remove(row);
    delete child;
    child = nullptr;
}


/**
  */
template<typename Derived>
void TreeNode<Derived>::removeChildren()
{
    // Temporarily make this "parent node defunct"
    defunct_ = true;

    // Release memory occupied by all the children
    qDeleteAll(children_);
    children_.clear();

    // Restore parent status
    defunct_ = false;

//    for (int i=children_.count()-1; i>= 0; --i)
//        removeChildAt(i);
}

/**
  */
template<typename Derived>
void TreeNode<Derived>::removeChildren(int row, int count)
{
    // Temporarily make this "parent node defunct"
    defunct_ = true;

    // Release memory occupied by the children between row and row + count - 1
    for (int i=0, j=row; i<count; ++i, ++j)
        delete children_.at(j);

    children_.remove(row, count);

    // Restore parent status
    defunct_ = false;
}

/**
  * [UNTESTED]
  * @param row [int]
  * @returns Derived *
  */
template<typename Derived>
Derived *TreeNode<Derived>::takeChildAt(int row)
{
    Q_ASSERT_X(row >= 0, "TreeNode<Derived>::takeChildAt", "row out of range; must be >= 0");
    Q_ASSERT_X(row <= children_.count(), "TreeNode<Derived>::takeChildAt", "row out of range; must be <= number of child nodes");

    // If you take a child, it's parent is set to 0 and it effectively becomes a root node
    Derived *node = children_.at(row);
    children_.remove(row);
    node->parent_ = nullptr;
    return node;
}

/**
  * Removes all children from this node without deallocating them, sets their parent to zero (effectively
  * making each one a root node), and returns this list.
  *
  * @returns QVector<Derived *>
  * @see takeChildAt()
  */
template<typename Derived>
QVector<Derived *> TreeNode<Derived>::takeChildren()
{
    QVector<Derived *> children = children_;
    foreach (Derived *child, children)
        child->parent_ = nullptr;

    children_.clear();

    return children;
}

/**
  * Removes count children beginning at row from this node without deallocating them, sets their parent to null
  * (effectively making each a root node), and returns them as a vector.
  *
  * @returns QVector<Derived *>
  * @see takeChildAt()
  */
template<typename Derived>
QVector<Derived *> TreeNode<Derived>::takeChildren(int row, int count)
{
    QVector<Derived *> children = children_.mid(row, count);
    children_.remove(row, count);
    foreach (Derived *child, children)
        child->parent_ = nullptr;

    return children;
}

#endif // TREENODE_H
