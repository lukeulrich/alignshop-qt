/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREEMODEL_H
#define ADOCTREEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QScopedPointer>

#include <QtCore/QMimeData>
#include <QtCore/QSet>

#include "../AdocTreeNode.h"

class ModelIndexMimeData : public QMimeData
{
public:
    QList<QModelIndex> indexes_;
private:
};

const int IsGroupRole = Qt::UserRole + 100;


/**
  * Owns and manages the data tree associated with an AlignShop document.
  *
  * AdocTreeModel provides the concrete implementation for interacting with the AlignShop data tree in
  * conjunction with the standard Qt view classes (e.g. QTreeView). It does so by providing hierarchical
  * access to the tree of AdocTreeNodes that comprise each and every AlignShop entity and ultimately the
  * AlignShop document itself.
  *
  * AdocTreeModel simply manages the hierarchical tree structure - it does not perform any node specific
  * functionality (e.g. deleting records, loading record data from database, etc.) This is all encapsulated
  * within specific child classes of AdocTreeNode.
  *
  * Testing is split between ModelTest and TestAdocTreeModel. ModelTest actively checks during debug usage
  * mode that the specific model operations perform as expected. While TestAdocTreeModel also tests the core
  * methods reimplemented from QAbstractItemModel, it also tests specific additional methods for manipulating
  * the data.
  *
  * Conceptually, AdocTreeModel is a simple tree model - it only contains a single column and many rows.
  * Related models may take this information and expand the number of columns by pulling this information
  * from the database or some other source.
  *
  * Upon instantiation, a default root node is created upon which everything else is placed.
  */
class AdocTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Roles
    enum UserRole {
        NodeTypeRole = Qt::UserRole + 1,
        AdocTreeNodeRole
    };

    // ------------------------------------------------------------------------------------------------
    // Constructors
    AdocTreeModel(QObject *parent = 0);                                         //!< Construct an instance of this class belonging to parent

    // ------------------------------------------------------------------------------------------------
    // Public methods - reimplemented parent class methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;           //!< Return the number of columns for the children of parent
    //! Returns the data stored under the given role for the item referred to by index
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;                //!< Configure the specific operations that may be performed on/by the item referred to by index
    //!< Returns the data for the given role and section in the header with the specified orientation
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    //! Return the QModelIndex at row and column that is a child of parent
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;                         //!< Return the parent QModelIndex of child
    int rowCount(const QModelIndex &parent = QModelIndex()) const;              //!< Return the number of rows for the children of parent
    //! Sets the data stored by the item referred to by index to value for role
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    // Drag and drop methods
    Qt::DropActions supportedDragActions() const;
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    // ------------------------------------------------------------------------------------------------
    // User public methods
    //! Append adocTreeNode as a child of parent and return true if this operation was successful or false otherwise. Assumes ownership of adocTreeNode
    bool appendRow(AdocTreeNode *adocTreeNode, const QModelIndex &parent = QModelIndex());
    //! Append all adocTreeNodes beneath parent and return true if this operation was successful or false otherwise. Assumes ownership of all adocTreeNodes
    bool appendRows(QList<AdocTreeNode *> adocTreeNodes, const QModelIndex &parent = QModelIndex());
    //! Performs a depth-first traversal of parents (inclusive) and returns a QStringList of identifiers for each NodeType excluding those that have a zero fkId_ value
    QHash<AdocTreeNode::NodeType, QStringList> getIdsByNodeType(QList<QModelIndex> parents) const;
    QModelIndex indexFromNode(AdocTreeNode *adocTreeNode) const;                //! Returns the QModelIndex of adocTreeNode on success or QModelIndex otherwise
    //! Insert adocTreeNode at row beneath parent and return true if this operation was successful or false otherwise. Assumes ownership of adocTreeNode
    bool insertRow(int row, AdocTreeNode *adocTreeNode, const QModelIndex &parent = QModelIndex());
    //! Returns true if nodeType is a valid child nodeType of parent; otherwise returns false [PARTIALLY TESTED]
    bool isValidChildType(AdocTreeNode::NodeType nodeType, const QModelIndex &parent = QModelIndex());
    AdocTreeNode *nodeFromIndex(const QModelIndex &index) const;                //!< Return a pointer to the AdocTreeNode referred to by index
    //! Remove count child rows beneath parent beginning with row inclusive
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    AdocTreeNode *root();                                                       //!< Return a raw pointer to the root node if any
    bool setRoot(AdocTreeNode *root);                                           //!< Set the root node to root and return true if this operation was successful or false otherwise; takes ownership of root

    // ------------------------------------------------------------------------------------------------
    // Helper document methods
    //! Appends a new group node labeled groupName under parent and returns its corresponding model index on success or an invalid model index otherwise
    QModelIndex newGroup(const QString &groupName, const QModelIndex &parent);

//    AdocDbDataSource adocDataSource_;     //!< Data source powering this model

    void dumpTree(AdocTreeNode *node, int level = 0);

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    QString formatDateTimeString(const QString &dateTimeString) const;          //!< Formats and returns dateTimeString as day Mon Year, hour:minute (a|p)m
    QString formatDateTimeString(const QVariant &dateTimeString) const;         //!< Formats and returns dateTimeString as day Mon Year, hour:minute (a|p)m
    static bool higherIndexRow(const QModelIndex &a, const QModelIndex &b);     //!< Returns true if b.row < a.row; false otherwise
    //! Core recursive function that aggregates the AdocTreeNode identifiers of parent and its children into result by NodeType (excluding nodes which have a zero fkId_ value)
    void recurseGetIdsByNodeType(AdocTreeNode *parent, QHash<AdocTreeNode::NodeType, QStringList> &result) const;

    // ------------------------------------------------------------------------------------------------
    // Private members
    QScopedPointer<AdocTreeNode> root_;  //!< Scoped pointer to root tree node

    //! Hash of hashes denoting which NodeTypes are allowed to have which NodeTypes
    static QHash<AdocTreeNode::NodeType, QHash<AdocTreeNode::NodeType, bool> > validDescendants_;

#ifdef TESTING
    // Provide access to the internal state of this class if we are testing
    friend class TestSliceProxyModel;    // specifically needed for calling reset on this model.
    friend class TestAdocTreeModel;
    friend class TestSliceModel;
#endif
};


#endif // ADOCTREEMODEL_H
