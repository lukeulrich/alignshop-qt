/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREENODE_H
#define ADOCTREENODE_H

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QString>

#include "TreeNode.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
// class AdocDbDataSource;

/**
  * Specialized TreeNode that represents any and all nodes within an AlignShop document tree
  *
  * AdocTreeNode extends TreeNode with three private members - a node type, label, and database identifier. All Adoc
  * entities that form part of the tree must have a corresponding node and node type.
  */
class AdocTreeNode : public TreeNode
{
    Q_ENUMS(NodeType)

public:
    // ------------------------------------------------------------------------------------------------
    // Enums
    enum NodeType {
        UndefinedType = 0,      //!< Catch-all node

        RootType,               //!< Tree root

        GroupType,              //!< Folder node for nested grouping

        SeqAminoType,           //!< Amino acid / protein sequence
        SeqDnaType,             //!< DNA sequence
        SeqRnaType,             //!< RNA sequence

        SubseqAminoType,        //!< Amino acid / protein subsequence
        SubseqDnaType,          //!< DNA subsequence
        SubseqRnaType,          //!< RNA subsequence

        MsaAminoType,           //!< Amino acid / protein alignment
        MsaDnaType,             //!< Dna alignment
        MsaRnaType,             //!< Rna alignment

        PrimerType,

        MaxNodeType             //!< Not a valid value to use, but useful for constraint checking
    };

    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Construct an AdocTreeNode with nodeType, and optional label, foreignTable, and fkId
    AdocTreeNode(NodeType nodeType = UndefinedType, const QString &label = QString(), int fkId = 0);
    explicit AdocTreeNode(const AdocTreeNode &other);                   //!< Shallow copy constructor that merely copies the data members in this class

    // ------------------------------------------------------------------------------------------------
    // Reimplmented public methods
    QList<AdocTreeNode *> childrenBetween(int start, int end) const;    //!< Convenience function that calls TreeNode::childreBetween and casts them to AdocTreeNode pointers

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Delete any database record for this and any descendant tree records and return true on success or false otherwise
//    bool eraseRecord(const AdocDbDataSource &dataSource);
    virtual bool isContainer() const;                                   //!< Returns true if this node is a container node

    // ------------------------------------------------------------------------------------------------
    // Static methods
    static NodeType nodeTypeEnum(const QString &string);                //!< Returns the enum corresponding to string or UndefinedNode if string is invalid
    static QString nodeTypeString(const NodeType &nodeType);            //!< Returns the QString representation of nodeType
    static bool isContainer(const NodeType &nodeType);                  //!< Retrusn true if nodeType is a container type or false otherwise

    NodeType nodeType_;         //!< Node type
    QString label_;             //!< Arbitrary label describing this node
    int fkId_;                  //!< Foreign key to foreign_table

protected:
    //! Private method for recursively deleting this and all child database records
//    virtual bool eraseRecordPrivate(const AdocDbDataSource &dataSource);

private:
    // Prevent the default assignment operator
    AdocTreeNode &operator=(const AdocTreeNode &other);


    static QHash<QString, NodeType> nodeTypeEnums_;     //!< Looking index that maps a QString to its equivalent NodeType
    static QList<QString> nodeTypeStrings_;             //!< Ordered list of NodeType equivalent QStrings

#ifdef TESTING
    friend class TestAdocTreeModel;
    friend class TestAdocTreeNode;
#endif
};

Q_DECLARE_METATYPE(AdocTreeNode *)
Q_DECLARE_METATYPE(AdocTreeNode::NodeType)

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param parent [AdocTreeNode *]
  * @param nodeType [NodeType]
  * @param label [QString]
  */
//inline
//AdocTreeNode::AdocTreeNode(NodeType nodeType, const QString &label, const QString &foreignTable, int fkId) :
//        TreeNode(), nodeType_(nodeType), label_(label), foreignTable_(foreignTable), fkId_(fkId)
//{
//}

/**
  * @param parent [AdocTreeNode *]
  * @param nodeType [NodeType]
  * @param label [QString]
  */
inline
AdocTreeNode::AdocTreeNode(NodeType nodeType, const QString &label, int fkId) :
        TreeNode(), nodeType_(nodeType), label_(label), fkId_(fkId)
{
}

/**
  * This method purely copies the public data members declared in this class. No deep copy or even a shallow
  * copy of the children_ or parent_ members (TreeNode) occurs. It is merely a convenience method for creating
  * a copy of this classes data members.
  *
  * @param other [const AdocTreeNode &]
  */
inline
AdocTreeNode::AdocTreeNode(const AdocTreeNode &other)
    : TreeNode(other), nodeType_(other.nodeType_), label_(other.label_), fkId_(other.fkId_)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
inline
bool AdocTreeNode::isContainer() const
{
    return isContainer(nodeType_);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static methods
/**
  * Simply utilizes an internal static hash to match various QStrings to their enum equivalent.
  *
  * @param string [const QString &]
  * @returns AdocTreeNode::NodeType
  */
inline
AdocTreeNode::NodeType AdocTreeNode::nodeTypeEnum(const QString &string)
{
    if (nodeTypeEnums_.contains(string))
        return nodeTypeEnums_.value(string);

    return AdocTreeNode::UndefinedType;
}

/**
  * First ensures that the nodeType will index valid position within the static QList of nodeTypeStrings_
  * and then simply returns the string at that index.
  *
  * @param nodeType [const AdocTreeNode::NodeType &]
  * @returns QString
  */
inline
QString AdocTreeNode::nodeTypeString(const NodeType &nodeType)
{
    Q_ASSERT_X(nodeTypeStrings_.count() == AdocTreeNode::MaxNodeType, "AdocTreeNode::nodeTypeString", "nodeTypeStrings_.count() does not equal AdocTreeNode::MAX");

    return nodeTypeStrings_.at(nodeType);
}


/**
  * @param nodeType [const AdocTreeNode::NodeType &]
  * @returns bool
  */
inline
bool AdocTreeNode::isContainer(const NodeType &nodeType)
{
    switch (nodeType)
    {
    case RootType:
    case GroupType:
    case MsaAminoType:
    case MsaDnaType:
    case MsaRnaType:
        return true;
    default:
        return false;
    }
}

#endif // ADOCTREENODE_H
