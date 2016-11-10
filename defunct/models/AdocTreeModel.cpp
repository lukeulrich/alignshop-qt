/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocTreeModel.h"

#include <QtCore/QDateTime>
#include <QtCore/QStack>

#include <QtGui/QFont>
#include <QtGui/QIcon>

#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlRecord>

#include <QtDebug>

#include "../global.h"

#include "../exceptions/DatabaseError.h"
#include "../exceptions/InvalidConnectionError.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Initialize the static rules specifying which NodeTypes are allowed have which NodeTypes
QHash<AdocTreeNode::NodeType, QHash<AdocTreeNode::NodeType, bool> > __initValidDescendants()
{
    QHash<AdocTreeNode::NodeType, QHash<AdocTreeNode::NodeType, bool> > validDescendants;

    // Root descendants
    validDescendants[AdocTreeNode::RootType][AdocTreeNode::GroupType] = true;
    validDescendants[AdocTreeNode::RootType][AdocTreeNode::SeqAminoType] = true;
    validDescendants[AdocTreeNode::RootType][AdocTreeNode::SeqDnaType] = true;
    validDescendants[AdocTreeNode::RootType][AdocTreeNode::SeqRnaType] = true;
    validDescendants[AdocTreeNode::RootType][AdocTreeNode::MsaAminoType] = true;
    validDescendants[AdocTreeNode::RootType][AdocTreeNode::MsaDnaType] = true;
    validDescendants[AdocTreeNode::RootType][AdocTreeNode::MsaRnaType] = true;
    validDescendants[AdocTreeNode::RootType][AdocTreeNode::PrimerType] = true;

    // Group descendants
    validDescendants[AdocTreeNode::GroupType][AdocTreeNode::GroupType] = true;
    validDescendants[AdocTreeNode::GroupType][AdocTreeNode::SeqAminoType] = true;
    validDescendants[AdocTreeNode::GroupType][AdocTreeNode::SeqDnaType] = true;
    validDescendants[AdocTreeNode::GroupType][AdocTreeNode::SeqRnaType] = true;
    validDescendants[AdocTreeNode::GroupType][AdocTreeNode::MsaAminoType] = true;
    validDescendants[AdocTreeNode::GroupType][AdocTreeNode::MsaDnaType] = true;
    validDescendants[AdocTreeNode::GroupType][AdocTreeNode::MsaRnaType] = true;
    validDescendants[AdocTreeNode::GroupType][AdocTreeNode::PrimerType] = true;

    // Seq descendants
    validDescendants[AdocTreeNode::SeqAminoType][AdocTreeNode::SubseqAminoType] = true;
    validDescendants[AdocTreeNode::SeqDnaType][AdocTreeNode::SubseqDnaType] = true;
    validDescendants[AdocTreeNode::SeqRnaType][AdocTreeNode::SubseqRnaType] = true;

    return validDescendants;
}
QHash<AdocTreeNode::NodeType, QHash<AdocTreeNode::NodeType, bool> > AdocTreeModel::validDescendants_(__initValidDescendants());


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Initialize with empty root node.
  *
  * @param parent [QObject *]
  */
AdocTreeModel::AdocTreeModel(QObject *parent) : QAbstractItemModel(parent), root_(new AdocTreeNode(AdocTreeNode::RootType, "Root node"))
{
}

// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * This tree model simply provides core data for all tree items and specifically the label. Thus, it
  * always returns 1.
  *
  * @param index [const QModelIndex &]
  * @returns int
  */
int AdocTreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

/**
  * TODO: Catch database errors.
  *
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant AdocTreeModel::data(const QModelIndex &index, int role) const
{
    AdocTreeNode *node = nodeFromIndex(index);
    if (!node)
        return QVariant();

    if (role == IsGroupRole)
        return node->nodeType_ == AdocTreeNode::GroupType;
    else if (role == NodeTypeRole)
        return qVariantFromValue(node->nodeType_);
    else if (role == AdocTreeNodeRole)
        return qVariantFromValue(node);

    if (index.column() != 0)
        return QVariant();

    if (role == Qt::DisplayRole)
        return node->label_;

    if (role == Qt::DecorationRole)
    {
        switch (node->nodeType_)
        {
        case AdocTreeNode::GroupType:
            if (index.parent().isValid())
                return QIcon(":icons/22x22/places/folder.png");
            else
                return QIcon(":icons/inkscape/22x22/document-new-project.png");
        case AdocTreeNode::SubseqAminoType:
            return QIcon(":icons/mine/22x22/amino-color.png");
        case AdocTreeNode::SubseqDnaType:
            return QIcon(":icons/mine/22x22/dna-grayscale.png");
        case AdocTreeNode::SubseqRnaType:
            return QIcon(":icons/mine/22x22/rna-stemloop-grayscale.png");
        case AdocTreeNode::MsaAminoType:
            return QIcon(":icons/mine/22x22/amino-alignment-color.png");
        case AdocTreeNode::MsaDnaType:
            return QIcon(":icons/mine/22x22/dna-alignment-grayscale.png");
        case AdocTreeNode::MsaRnaType:
            return QIcon(":icons/mine/22x22/rna-alignment-grayscale.png");
        default:
            return QVariant();
        }
    }

    return QVariant();
}

/**
  * Only groups are valid drop targets; all items are draggable.
  *
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags AdocTreeModel::flags(const QModelIndex &index) const
{
    AdocTreeNode *node = nodeFromIndex(index);
    if (!node || index.column() != 0 || !index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    if (node->nodeType_ == AdocTreeNode::GroupType)
        flags |= Qt::ItemIsDropEnabled;

    return flags;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant AdocTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:
            return "Name";
        default:
            return QVariant();
        }
    }
    else // orientation == Qt::Vertical; use this to indicate the row number. Section is 0-based, so we add 1 to make it 1-based.
        return section + 1;
}

/**
  * AdocTreeModel only supports one column of data which must be zero and the row value must
  * be within a valid range.
  *
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex AdocTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return QModelIndex();

    if (column != 0 || row < 0 || row >= parentNode->childCount())
        return QModelIndex();

    return createIndex(row, column, parentNode->childAt(row));
}

/**
  * @param index [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex AdocTreeModel::parent(const QModelIndex &index) const
{
    AdocTreeNode *childNode = nodeFromIndex(index);
    if (!childNode)
        return QModelIndex();

    TreeNode *parentNode = childNode->parent();
    if (!parentNode || parentNode == root_.data())
        return QModelIndex();

    if (!parentNode->parent())
        return QModelIndex();

    return createIndex(parentNode->row(), 0, parentNode);
}

/**
  * Simply the number of TreeNode children beneath the TreeNode pointer for index.
  *
  * @param index [const QModelIndex &]
  * @returns int
  */
int AdocTreeModel::rowCount(const QModelIndex &index) const
{
    // No need to test that the index column is zero, because:
    // 1) the index function does not create valid indices without this value
    // 2) the nodeFromIndex function ensures that the index belongs to this model (among other things).
    AdocTreeNode *parentNode = nodeFromIndex(index);
    if (parentNode)
        return parentNode->childCount();

    return 0;
}

/**
  * Currently, the only supported role is Qt::EditRole and this directly impacts the node label
  *
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  */
bool AdocTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole
        || !index.isValid()
        || index.column() != 0
        || value.toString().isEmpty())
    {
        return false;
    }

    // Also checks that index belongs to this model
    AdocTreeNode *node = nodeFromIndex(index);
    if (!node)
        return false;

    // This may be unnecessary given that data assignment is largely handled by downstream models
    // (e.g. slice models and container filter model)
    if (node->nodeType_ == AdocTreeNode::GroupType
        || node->nodeType_ == AdocTreeNode::SubseqAminoType
        || node->nodeType_ == AdocTreeNode::SubseqDnaType
        || node->nodeType_ == AdocTreeNode::SubseqRnaType)
    {
        node->label_ = value.toString();
        emit dataChanged(index, index);
        return true;
    }

    return false;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Drag and drop methods
/**
  * @returns Qt::DropActions
  */
Qt::DropActions AdocTreeModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

/**
  * @returns Qt::DropActions
  */
Qt::DropActions AdocTreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

/**
  * @returns QStringList
  */
QStringList AdocTreeModel::mimeTypes() const
{
    return QStringList() << "application/x-alignshop";
}

/**
  * @param indexes [const QModelIndexList &]
  * @returns QMimeData *
  */
QMimeData *AdocTreeModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return 0;

    ModelIndexMimeData *mimeData = new ModelIndexMimeData();
    mimeData->indexes_ = indexes;
    mimeData->setData("application/x-alignshop", QByteArray());
    return mimeData;
}

/**
  * When this method is called via a QAbstractItemView with MoveAction and true is returned, the
  * item view will call removeRows if its dragDropOverwriteMode is false. It is vital that this property
  * be false for any view that it is attached to this model otherwise the tree will contain duplicate
  * TreeNode pointers and will crash when the program exits.
  *
  * Constraints:
  * o action must be Qt::MoveAction as that is the only drop operation supported
  * o data must polymorphically point to a ModelIndexMimeData instance
  * o row and column must both be -1 - drops between or around items are not supported
  * o parent must either be a group or root node
  *
  * o No items to be moved may be the parent of any other item being moved. In other words,
  *   In the example below, GroupA and GroupB are in the same list to be dropped. This is not allowed
  *   and will return false.
  *
  *   GroupA       (in data)
  *   |___ GroupB  (in data)
  *   |___ NodeA
  *
  *   Why? It is unclear how to handle this situation (or why this situation would be desired for internal
  *   moves). If this were permitted, where should NodeA be placed? Why?
  *
  * IMPORTANT: Any attached item view must have its dragDropOverwriteMode set to false or otherwise
  *            duplicate tree pointers will exist and result in a program crash on termination!
  *
  * @param data [const QMimeData *]
  * @param action [Qt::DropAction]
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!data)
    {
        qWarning("dropMimeData called with null data pointer");
        return false;
    }

    if (!data->hasFormat("application/x-alignshop"))
        return false;

    // -1 for both column and row indicates that these items were dropped onto a parent
    // Only consider drops that occur on an item and not elsewhere
    if (column != -1 || row != -1)
        return false;

    if (action != Qt::MoveAction)
        return false;

    // Get the parent node
    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return false;

    if (parentNode->nodeType_ != AdocTreeNode::GroupType
        && parentNode->nodeType_ != AdocTreeNode::RootType)
        return false;

    // Attempt to convert data to a ModelIndexMimeData pointer
    const ModelIndexMimeData *mimeData = dynamic_cast<const ModelIndexMimeData *>(data);
    Q_ASSERT_X(mimeData, "AdocTreeModel::dropMimeData", "data must be a base class pointer to a ModelIndexMimeData instance");
    if (!mimeData)
        return false;

    // and ensure that the model index list contains at least one entry
    Q_ASSERT_X(mimeData->indexes_.count(), "AdocTreeModel::dropMimeData", "no indexes defined provided in data paramater");

    // Store the nodes to be appended in a list and after looping through all indices and confirming
    // that none of their immediate parents are equivalent to the parent parameter passed to this method,
    // then continue with the addition. Otherwise, return false.
    //
    // Additionally check that none of the indexes are parents of any of the other indexes to be moved.
    QList<AdocTreeNode *> nodesToMove;
    foreach (const QModelIndex &index, mimeData->indexes_)
    {
        AdocTreeNode *node = nodeFromIndex(index);
        if (!node)
        {
            qWarning("AdocTreeModel::dropMimeData - invalid index");
            return false;
        }

        if (index == parent)
        {
            qWarning("AdocTreeModel::dropMimeData - dropping index onto currently selected node is not allowed");
            return false;
        }

        // Enforce constraint that all nodes to be moved must not be immediate children of parent
        if (index.parent() == parent)
            return false;

        // Make sure that index (node) is not an ancestor of parent (parentNode)
        // For instance to prevent the following or similar scenario from occurring:
        //
        // Root
        // |___ Alpha (group)      <-- Selected item = node
        //      |___ Beta (group)  <-- Destination = parentNode
        //
        // Alpha may not be moved into beta because it is an ancestor of beta. This while loop
        // checks for this condition and returns false if encountered.
        if (parentNode->isDescendantOf(node))
            return false;

        // Make sure that none of the indexes being moved are descendants of any other items
        // Ideally this should not happen because any attached QTreeViews should only have single
        // selection. Tested in TestSliceModel although really should be performed in TestAdocTreeModel.
        foreach (const AdocTreeNode *nodeToMove, nodesToMove)
            if (nodeToMove->isDescendantOf(node) || node->isDescendantOf(nodeToMove))
                return false;

        nodesToMove << node;
    }

    // At this point, we are guaranteed to have a list of valid nodes that should be moved.
    //
    // Because the MVC/DND framework is tailored to work with any number of models and views, we
    // create a new AdocTreeNode for each node to be moved and simply transfer all of the original
    // children to this new node. Upon returning true, the calling view will handle removing the
    // selected rows.
    QList<AdocTreeNode *> nodesToAppend;    // List of newly created nodes with identical member values to those nodes that
                                            // are to be moved.

    foreach (AdocTreeNode *nodeToMove, nodesToMove)
    {
        // Create new node that is a shallow copy of nodeToMove
        AdocTreeNode *newNode = new AdocTreeNode(*nodeToMove);

        // Manually transfer all children of nodeToMove to this newNode
        if (nodeToMove->childCount())
        {
            beginRemoveRows(indexFromNode(nodeToMove), 0, nodeToMove->childCount()-1);
            newNode->appendChildren(nodeToMove->takeChildren());
            endRemoveRows();
        }

        // To prevent this node from being removed from the database, we set its fkId_ to zero. How does this work?
        // When removeRows is called it recursively collects all nonzero fkId_ values for a given node. By setting the
        // fkId_ to zero, it will effectively skip over this record when communicating to the database.
        nodeToMove->fkId_ = 0;

        nodesToAppend << newNode;
    }

    // Append them to parent
    appendRows(nodesToAppend, parent);

    return true;
}


void AdocTreeModel::dumpTree(AdocTreeNode *node, int level)
{
    for (int i=0, z= node->childCount(); i<z; ++i)
    {
        AdocTreeNode *child = static_cast<AdocTreeNode *>(node->childAt(i));
        qDebug() << QString("\t").repeated(level) << child->label_;
        dumpTree(child, level+1);
    }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &]
  * @param adocTreeNode [AdocTreeNode *]
  */
bool AdocTreeModel::appendRow(AdocTreeNode *adocTreeNode, const QModelIndex &parent)
{
    Q_ASSERT_X(adocTreeNode, "AdocTreeModel::appendRow", "adocTreeNode must not be empty");

    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return false;

    int row = parentNode->childCount();

    beginInsertRows(parent, row, row);
    parentNode->appendChild(adocTreeNode);
    endInsertRows();

    return true;
}

/**
  * [UNTESTED]
  *
  * @param parent [const QModelIndex &]
  * @param adocTreeNode [AdocTreeNode *]
  */
bool AdocTreeModel::appendRows(QList<AdocTreeNode *> adocTreeNodes, const QModelIndex &parent)
{
    Q_ASSERT_X(adocTreeNodes.count(), "AdocTreeModel::appendRow", "adocTreeNodes must not be empty");

    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return false;

    int row = parentNode->childCount();

    beginInsertRows(parent, row, row + adocTreeNodes.count() - 1);
    for (int i=0, z=adocTreeNodes.count(); i<z; ++i)
        parentNode->appendChild(adocTreeNodes.at(i));
    endInsertRows();

    return true;
}

/**
  * Iterates through each parent and recursively groups the various nodeType identifiers into a QStringList.
  * This is useful for bulk managing the records in the database versus single row changes.
  *
  * If duplicate parent items are passed to this method, there will be duplicate identifiers returned.
  *
  * Any indices originating from another model are ignored.
  *
  * @param parents [QList<QModelIndex>]
  * @returns QHash<AdocTreeNode::NodeType, QStringList>
  */
QHash<AdocTreeNode::NodeType, QStringList> AdocTreeModel::getIdsByNodeType(QList<QModelIndex> parents) const
{
    QHash<AdocTreeNode::NodeType, QStringList> idsByNodeType;
    foreach (const QModelIndex &parent, parents)
    {
        AdocTreeNode *parentNode = nodeFromIndex(parent);
        if (!parentNode)
            continue;

        recurseGetIdsByNodeType(parentNode, idsByNodeType);
    }

    return idsByNodeType;
}

/**
  * Performs the opposite of nodeFromIndex by walking up the TreeNode structure to the root,
  * and then mapping back to the supplied adocTreeNode using the sourceModel index function.
  *
  * @param adocTreeNode [AdocTreeNode *]
  * @returns QModelIndex
  */
QModelIndex AdocTreeModel::indexFromNode(AdocTreeNode *adocTreeNode) const
{
    Q_ASSERT_X(adocTreeNode, "AdocTreeModel::indexFromNode", "adocTreeNode pointer must not be null");
    if (!adocTreeNode)      // Release mode guard
        return QModelIndex();

    // Check if adocTreeNode is the root
    if (!adocTreeNode->parent())
        return QModelIndex();

    TreeNode *node = adocTreeNode;
    QStack<TreeNode *> nodeStack;
    while (node->parent())
    {
        nodeStack.push(node);
        node = node->parent();
    }

    // At the root node, now work backwards through stack until we get the model index of the parent of pointerList_.first (thus
    // the z=...-1
    QModelIndex parentIndex = QModelIndex();
    for (int i=0, z=nodeStack.count()-1; i<z; ++i)
    {
        node = nodeStack.pop();
        parentIndex = index(node->row(), 0, parentIndex);
    }

    return index(adocTreeNode->row(), 0, parentIndex);
}

/**
  * @param row [int]
  * @param parent [const QModelIndex &]
  * @param adocTreeNode [AdocTreeNode *]
  * @returns bool
  */
bool AdocTreeModel::insertRow(int row, AdocTreeNode *adocTreeNode, const QModelIndex &parent)
{
    Q_ASSERT_X(adocTreeNode, "AdocTreeModel::appendRow", "adocTreeNode must not be empty");

    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return false;

    Q_ASSERT_X(row >= 0, "AdocTreeModel::insertRow", QString("row out of range, must be >= 0 (invalid row value: %1)").arg(row).toAscii());
    Q_ASSERT_X(row <= parentNode->childCount(), "AdocTreeModel::insertRow", QString("row out of range, must be < parentNode->childCount() (%1) (invalid row value: %2)").arg(parentNode->childCount()).arg(row).toAscii());

    beginInsertRows(parent, row, row);
    parentNode->insertChildAt(row, adocTreeNode);
    endInsertRows();

    return true;
}

/**
  * Obtains the nodeType for parent and returns true if this parent NodeType / child NodeType association is
  * present within the validDescandants hash.
  *
  * @param nodeType [AdocTreeNode::NodeType]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeModel::isValidChildType(AdocTreeNode::NodeType nodeType, const QModelIndex &parent)
{
    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (parentNode &&
        validDescendants_.contains(parentNode->nodeType_) &&
        validDescendants_[parentNode->nodeType_].contains(nodeType))
    {
        return true;
    }

    return false;
}


/**
  * If index is valid returns a raw pointer to the AdocTreeNode referred to by index. Otherwise, returns
  * the raw pointer to the root node. The root node pointer may be null if root_ is cleared.
  *
  * If index is valid and does not belong to this model, a null pointer is returned.
  *
  * @param index [const QModelIndex &]
  * @returns AdocTreeNode *
  */
AdocTreeNode *AdocTreeModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (index.model() != this)
        {
            qWarning("AdocTreeModel::nodeFromIndex - %s", "wrong index passed to model");
            return 0;
        }

        return static_cast<AdocTreeNode *>(index.internalPointer());
    }

    return root_.data();
}

/**
  * Serves the dual function of deleting database rows corresponding to the given data_tree items
  * and also removes the items from the data_tree as well. Because of the hierarchical nature of
  * the data tree, it is necessary to recursively iterate through all descendants for records that
  * should be removed - not just the parent rows. Only those records that have a valid foreign key
  * value will be considered.
  *
  * Note: Some tests (notably SliceModel) use an artifical AdocTreeModel that is not attached to a
  *       real SQLite database. Consequently, any method that issues a database request will fail
  *       and specifically, DbDataSource (or its subclasses) methods that throw an exception will
  *       cause this function to return false. Such tests will then likely fail because they expect
  *       to update their state based on signals emitted from this method. The mechanism by which
  *       the tests function as expected is to not set the fkId_ for each node when constructing a
  *       dummy source tree for the AdocTreeModel. Thus, the getIdsByNodeType() method will return
  *       an empty hash and no database methods will be called. See AdocDbDataSource::eraseRecords().
  *
  * @param row [int]
  * @param count [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return false;

    Q_ASSERT_X(parentNode->childCount() > 0, "AdocTreeModel::removeRows",
               "parentNode does not have any children to remove");

    Q_ASSERT_X(row >= 0, "AdocTreeModel::removeRows",
               QString("row out of range, must be >= 0 (invalid row value: %1)").arg(row).toAscii());

    Q_ASSERT_X(row < parentNode->childCount(), "AdocTreeModel::removeRows",
               QString("row out of range, must be < parentNode->childCount (%1) (invalid row value: %2)").arg(parentNode->childCount()).arg(row).toAscii());

    Q_ASSERT_X(count >= 0, "AdocTreeModel::removeRows",
               QString("count must be at least 0 (invalid count value: %1)").arg(count).toAscii());

    Q_ASSERT_X(row + count <= parentNode->childCount(), "AdocTreeModel::removeRows",
               QString("row (%1) + count (%2) must be <= parentNode.childCount (%3)").arg(row).arg(count).arg(parentNode->childCount()).toAscii());

    // In debug, count will always be greater than or equal to 0; however, in release mode when the asserts are disabled,
    // do nothing if count is negative
    if (count < 0)
        return false;

    // If there are no rows to remove, simply return true
    if (count == 0)
        return true;

    // In debug mode, parentNode->childCount() will always be a positive number or throw an assertion; however, in release mode
    //   with asserts turned off, it is possible to call this function with a parent that has no children. Ignore these requests
    if (parentNode->childCount() == 0)
        return false;

    // Attempt to delete all corresponding database records
    try
    {
        // Build a list of indices corresponding to the rows to be removed.
        QList<QModelIndex> indicesToRemove;
        for (int i=row, z= row+count; i<z; ++i)
            indicesToRemove.append(parent.child(i, 0));

        QHash<AdocTreeNode::NodeType, QStringList> idsByNodeType = getIdsByNodeType(indicesToRemove);
//        adocDbDataSource_.eraseRecords(idsByNodeType);
    }
    catch (InvalidConnectionError &e)
    {
        // TODO: report error
        return false;
    }
    catch (DatabaseError &e)
    {
        // TODO: report error
        return false;
    }
    catch (...)
    {
        // TODO: report error
        return false;
    }

    // Successfully removed the rows from the database, now remove them from the in-memory tree structure
    beginRemoveRows(parent, row, row + count - 1);
    for (int i=row, z= row + count; i<z; ++i)
    {
        // Use a dynamic cast to make sure that the node we are removing is indeed an AdocTreeNode
        AdocTreeNode *childNode = dynamic_cast<AdocTreeNode *>(parentNode->childAt(row));
        Q_ASSERT_X(childNode, "AdocTreeModel::removeRows", QString("Null pointer obtained for childAt %1").arg(row).toAscii());

        parentNode->removeChildAt(row);
    }
    endRemoveRows();

    return true;
}

/**
  * @returns AdocTreeNode *
  */
AdocTreeNode *AdocTreeModel::root()
{
    return root_.data();
}

/**
  * Updates the root node for this tree to root - if it contains a non-null pointer. Also flushes
  * the seqIdToTreeNodeHash_ index because the model has been reset.
  *
  * @param root [QSharedPointer<AdocTreeNode>]
  * @returns bool
  */
bool AdocTreeModel::setRoot(AdocTreeNode *root)
{
    if (!root)
        return false;

    beginResetModel();
    root_.reset(root);
    endResetModel();

    return true;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper document methods
/**
  * A GroupType node must be permitted beneath parent, otherwise an invalid QModelIndex will be returned.
  *
  * @param groupName [const QString &]
  * @param QModelIndex
  */
QModelIndex AdocTreeModel::newGroup(const QString &groupName, const QModelIndex &parent)
{
    if (isValidChildType(AdocTreeNode::GroupType, parent))
    {
        AdocTreeNode *group = new AdocTreeNode(AdocTreeNode::GroupType, groupName);
        if (appendRow(group, parent))
            return index(rowCount(parent)-1, 0, parent);

        // Otherwise, failed to append the group to the model
        delete group;
        group = 0;
    }

    return QModelIndex();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Returns the same string if unable to convert it to a valid QDateTime object.
  *
  * @param dateTimeString [const QString &]
  * @returns QString
  */
QString AdocTreeModel::formatDateTimeString(const QString &dateTimeString) const
{
    QDateTime time = QDateTime::fromString(dateTimeString, Qt::ISODate);
    if (!time.isValid())
        return dateTimeString;

    return time.toString("d MMM yyyy, h:mm ap");
}

QString AdocTreeModel::formatDateTimeString(const QVariant &dateTimeString) const
{
    return formatDateTimeString(dateTimeString.toString());
}

bool AdocTreeModel::higherIndexRow(const QModelIndex &a, const QModelIndex &b)
{
    return b.row() < a.row();
}

/**
  * Only selects those nodes that have a non-zero fkId_.
  *
  * @param node [AdocTreeNode *]
  * @param result [QHash<AdocTreeNode::NodeType, QStringList>]
  * @see getIdsByNodeType()
  */
void AdocTreeModel::recurseGetIdsByNodeType(AdocTreeNode *node, QHash<AdocTreeNode::NodeType, QStringList> &result) const
{
    Q_ASSERT_X(node, "AdocTreeModel::recurseGetIdsByNodeType", "node pointer must not be null");
    if (!node)    // Release mode guard
        return;

    for (int i=0, z=node->childCount(); i<z; ++i)
        recurseGetIdsByNodeType(static_cast<AdocTreeNode *>(node->childAt(i)), result);

    if (node->fkId_)
        result[node->nodeType_].append(QString::number(node->fkId_));
}
