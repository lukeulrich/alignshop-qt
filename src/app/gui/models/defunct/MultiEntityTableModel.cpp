/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "MultiEntityTableModel.h"

#include "AdocTreeModel.h"
#include "AdocTreeModelRoles.h"
#include "ColumnAdapters/IColumnAdapter.h"
#include "../../core/Entities/IEntity.h"
#include "../../core/Repositories/IRepository.h"
#include "../../core/AdocNodeData.h"
#include "../../core/TreeNode.h"
#include "../../core/macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
MultiEntityTableModel::MultiEntityTableModel(QObject *parent) :
    QAbstractTableModel(parent), adocTreeModel_(nullptr), root_(nullptr)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int MultiEntityTableModel::columnCount(const QModelIndex & /* parent */) const
{
    return eNumberOfColumns;
}

/**
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant MultiEntityTableModel::data(const QModelIndex &index, int role) const
{
    ASSERT(index.isValid());
    ASSERT(index.column() < eNumberOfColumns);
    if (role == Qt::DisplayRole)
    {
        if (!isGroupIndex(index))
        {
            IEntity *entity = entityFromIndex(index);
            if (!entityTypeColumnAdapterHash_.contains(entity->type()))
                return QVariant();

            const EntityAdapterSpecification &specification = entityTypeColumnAdapterHash_.value(entity->type());
            ASSERT(specification.columnAdapter_ != nullptr);

            int mappedColumn = specification.columnMap_[index.column()];
            if (mappedColumn == -1)
                return QVariant();

            return specification.columnAdapter_->data(entity, mappedColumn);
        }

        // Got a group index
        if (index.column() == 0)
            return groups_.at(index.row())->data_.label_;
    }
    else if (role == CustomRoles::kIsGroupRole)
    {
        return isGroupIndex(index);
    }

    return QVariant();

}

/**
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags MultiEntityTableModel::flags(const QModelIndex &index) const
{
    if (!isGroupIndex(index))
    {
        IEntity *entity = entityFromIndex(index);
        ASSERT(entityTypeColumnAdapterHash_.contains(entity->type()));

        const EntityAdapterSpecification &specification = entityTypeColumnAdapterHash_.value(entity->type());
        ASSERT(specification.columnAdapter_ != nullptr);

        int mappedColumn = specification.columnMap_[index.column()];
        if (mappedColumn == -1)
            return 0;

        return specification.columnAdapter_->flags(mappedColumn);
    }

    // index is a group index
    if (index.column() == 0)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    // Permit nothing to be done with the remaining columns
    return 0;
}

/**
  * @returns AdocTreeNode *
  */
AdocTreeNode *MultiEntityTableModel::root() const
{
    return root_;
}

/**
  * @returns QModelIndex
  */
QModelIndex MultiEntityTableModel::rootIndex() const
{
    return rootIndex_;
}

/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int MultiEntityTableModel::rowCount(const QModelIndex & /* parent */) const
{
    return groups_.size() + entities_.size();
}

/**
  * @param entityType [int]
  * @param entityAdapterSpecification [const EntityAdapterSpecification &]
  */
void MultiEntityTableModel::setAdapterSpecification(int entityType, const EntityAdapterSpecification &entityAdapterSpecification)
{
    ASSERT(entityAdapterSpecification.columnAdapter_ != nullptr);

    if (entityTypeColumnAdapterHash_.contains(entityType))
    {
        IColumnAdapter *oldAdapter = entityTypeColumnAdapterHash_.value(entityType).columnAdapter_;
        disconnect(oldAdapter, SIGNAL(dataChanged(IEntity*,int)), this, SLOT(onEntityDataChanged(IEntity*,int)));
        // No need to remove the old specification since it will be replaced with the new entityAdapterSpecification
    }

    entityTypeColumnAdapterHash_.insert(entityType, entityAdapterSpecification);

    connect(entityAdapterSpecification.columnAdapter_,
            SIGNAL(dataChanged(IEntity*,int)),
            SLOT(onEntityDataChanged(IEntity*,int)));
}

/**
  * @param adocTreeModel [AdocTreeModel *]
  */
void MultiEntityTableModel::setSourceTreeModel(AdocTreeModel *adocTreeModel)
{
    beginResetModel();

    if (adocTreeModel_)
    {
        disconnect(adocTreeModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onTreeModelDataChanged(QModelIndex,QModelIndex)));
        disconnect(adocTreeModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onTreeModelRowsInserted(QModelIndex,int,int)));
        disconnect(adocTreeModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(onTreeModelRowsAboutToBeRemoved(QModelIndex,int,int)));
    }

    adocTreeModel_ = adocTreeModel;
    resetVariables();

    if (adocTreeModel_ != nullptr)
    {
        connect(adocTreeModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onTreeModelDataChanged(QModelIndex,QModelIndex)));
        connect(adocTreeModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onTreeModelRowsInserted(QModelIndex,int,int)));
        connect(adocTreeModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(onTreeModelRowsAboutToBeRemoved(QModelIndex,int,int)));
    }

    endResetModel();
}

/**
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  * @returns bool
  */
bool MultiEntityTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (!isGroupIndex(index))
    {
        IEntity *entity = entityFromIndex(index);
        ASSERT(entityTypeColumnAdapterHash_.contains(entity->type()));

        const EntityAdapterSpecification &specification = entityTypeColumnAdapterHash_.value(entity->type());
        ASSERT(specification.columnAdapter_ != nullptr);

        int mappedColumn = specification.columnMap_[index.column()];
        if (mappedColumn == -1)
            return false;

        return specification.columnAdapter_->setData(entity, mappedColumn, value);
    }

    // Dealing with a group index
    if (index.column() == 0)
    {
        // Map back to the original tree model
        AdocTreeNode *groupNode = groups_.at(index.row());
        return adocTreeModel_->setData(adocTreeModel_->indexFromNode(groupNode), value, role);
    }

    return false;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param rootIndex [const QModelIndex &]
  */
void MultiEntityTableModel::setRoot(const QModelIndex &rootIndex)
{
    ASSERT(adocTreeModel_ != nullptr);
    ASSERT(rootIndex.isValid() == false || rootIndex.model() == adocTreeModel_);

    beginResetModel();
    resetVariables();

    rootIndex_ = rootIndex;
    AdocTreeNode *newRoot = adocTreeModel_->nodeFromIndex(rootIndex);

    root_ = newRoot;
    if (root_ != nullptr)
        findAddNodes(extractAcceptableNodes(newRoot));

    endResetModel();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param nodeType [int]
  * @returns bool
  */
bool MultiEntityTableModel::acceptNodeType(int nodeType) const
{
    return nodeType == eGroupNode || entityTypeColumnAdapterHash_.contains(nodeType);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param topLeft [const QModelIndex &]
  * @param bottomRight [const QModelIndex &]
  */
void MultiEntityTableModel::onTreeModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    ASSERT(topLeft.isValid());
    ASSERT(topLeft.model() == adocTreeModel_);
    ASSERT(topLeft == bottomRight);
    ASSERT(adocTreeModel_ != nullptr);

    // Ignore all changes outside of the currently modeled node
    if (topLeft.parent() != rootIndex_)
        return;

    AdocTreeNode *groupNode = adocTreeModel_->nodeFromIndex(topLeft);
    ASSERT(groupNode != nullptr);
    ASSERT(groupNode->data_.nodeType_ == eGroupNode);
    int i = groups_.indexOf(groupNode);
    if (i != -1)
    {
        QModelIndex changedIndex = QAbstractTableModel::index(i, 0);
        emit dataChanged(changedIndex, changedIndex);
    }
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void MultiEntityTableModel::onTreeModelRowsInserted(const QModelIndex &parent, int start, int end)
{
    ASSERT(parent.isValid() == false || parent.model() == adocTreeModel_);
    if (root_ == nullptr || parent != rootIndex_)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(parent);
    QHash<AdocNodeType, AdocTreeNodeVector> acceptedNodes = extractAcceptableNodes(parentNode, start, end);
    if (acceptedNodes.isEmpty())
        return;

    // Now tell connected components that we are about to add some rows
    beginInsertRows(QModelIndex(), entities_.size(), entities_.size() + sumAcceptableNodes(acceptedNodes) - 1);
    findAddNodes(acceptedNodes);
    endInsertRows();
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void MultiEntityTableModel::onTreeModelRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    ASSERT(parent.isValid() == false || parent.model() == adocTreeModel_);
    if (root_ == nullptr)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(parent);

    // Case 1: parentNode == root
    if (root_ == parentNode)
    {
        QVector<IEntity *>::Iterator it = entities_.begin();
        while (it != entities_.constEnd())
        {
            bool foundMatch = false;

            IEntity *entity = *it;
            for (int i=start; i<=end; ++i)
            {
                AdocTreeNode *child = parentNode->childAt(i);
                if (child->data_.entityId_ == entity->id() &&
                    child->data_.nodeType_ == entity->type())
                {
                    // We got ourselves a match! Remove this puppy!
                    int row = it - entities_.constBegin();
                    beginRemoveRows(QModelIndex(), row, row);
                    repositoryForNodeType(child->data_.nodeType_)->unfindOne(entity);
                    endRemoveRows();

                    foundMatch = true;

                    break;
                }
            }

            if (!foundMatch)
                ++it;
            else
                it = entities_.erase(it);
        }

        return;
    }

    // Case 2: root is a descendant of one of the rows being removed
    for (int i=start; i<=end; ++i)
    {
        if (root_->isDescendantOf(parentNode->childAt(i)))
        {
            // The whole shabang is being removed
            beginResetModel();
            resetVariables();
            endResetModel();
            return;
        }
    }

    // Case 3: Removal is outside the context of the node currently being modeled - do nothing
}

/**
  * @param entity [IEntity *]
  * @param column [int]
  */
void MultiEntityTableModel::onEntityDataChanged(IEntity *entity, int column)
{
    ASSERT(entity != nullptr);
    ASSERT(column >= 0);

    int entityRow = entities_.indexOf(entity);
    if (entityRow == -1)
        return;

    ASSERT(entityTypeColumnAdapterHash_.contains(entity->type()));
    const EntityAdapterSpecification &specification = entityTypeColumnAdapterHash_.value(entity->type());
    for (int i=0; i< eNumberOfColumns; ++i)
    {
        if (column == specification.columnMap_[i])
        {
            QModelIndex changedIndex = QAbstractTableModel::index(groups_.size() + entityRow, i);
            emit dataChanged(changedIndex, changedIndex);
            return;
        }
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param index [const QModelIndex &index]
  * @returns IEntity *
  */
IEntity *MultiEntityTableModel::entityFromIndex(const QModelIndex &index) const
{
    ASSERT(isGroupIndex(index) == false);
    ASSERT(entities_.size() > 0);
    ASSERT(index.model() == this);
    ASSERT(index.isValid());

    IEntity *entity = entities_.at(index.row() - groups_.size());
    ASSERT(entity != nullptr);

    return entity;
}

/**
  * @param parent [const AdocTreeNode *]
  * @returns QHash<AdocNodeType, AdocTreeNodeVector>
  */
QHash<AdocNodeType, AdocTreeNodeVector> MultiEntityTableModel::extractAcceptableNodes(const AdocTreeNode *parent) const
{
    if (parent->childCount() > 0)
        return extractAcceptableNodes(parent, 0, parent->childCount() - 1);

    return QHash<AdocNodeType, AdocTreeNodeVector>();
}

/**
  * @param parent [const AdocTreeNode *]
  * @param start [int]
  * @param end [int]
  * @returns QHash<AdocNodeType, AdocTreeNodeVector>
  */
QHash<AdocNodeType, AdocTreeNodeVector> MultiEntityTableModel::extractAcceptableNodes(const AdocTreeNode *parent, int start, int end) const
{
    ASSERT(parent != nullptr);
    ASSERT(start >= 0 && start <= end);
    ASSERT(end < parent->childCount());

    QHash<AdocNodeType, AdocTreeNodeVector> acceptableNodeHash; // {node type -> [node *] }
    for (int i=start; i<=end; ++i)
    {
        AdocTreeNode *child = parent->childAt(i);
        if (acceptNodeType(child->data_.nodeType_))
            acceptableNodeHash[child->data_.nodeType_] << child;
    }
    return acceptableNodeHash;
}

/**
  * @param entityNodeHash [const QHash<AdocNodeType, AdocTreeNodeVector> &]
  */
void MultiEntityTableModel::findAddNodes(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash)
{
    QHash<AdocNodeType, AdocTreeNodeVector>::ConstIterator it = entityNodeHash.constBegin();
    for (; it != entityNodeHash.constEnd(); ++it)
    {
        // it.key() = node type
        // it.value() = AdocTreeNodeVector
        if (it.key() == eGroupNode)
        {
            groups_ << it.value();
            continue;
        }

        ASSERT(it.key() != eRootNode);

        // Must be entity node
        IRepository *repository = repositoryForNodeType(it.key());
        ASSERT(repository != nullptr);
        const AdocTreeNodeVector &adocTreeNodeVector = it.value();
        QVector<int> entityIds;
        entityIds.reserve(adocTreeNodeVector.size());
        for (int i=0, z=it.value().size(); i<z; ++i)
            entityIds << adocTreeNodeVector.at(i)->data_.entityId_;

        entities_ << repository->findGeneric(entityIds);
    }
}

/**
  * @param index [const QModelIndex &]
  * @returns bool
  */
bool MultiEntityTableModel::isGroupIndex(const QModelIndex &index) const
{
    ASSERT(index.isValid());

    return index.row() < groups_.size();
}

/**
  * @param nodeType [AdocNodeType]
  * @returns IRepository *
  */
IRepository *MultiEntityTableModel::repositoryForNodeType(AdocNodeType nodeType) const
{
    if (!entityTypeColumnAdapterHash_.contains(nodeType))
        return nullptr;

    ASSERT(entityTypeColumnAdapterHash_.value(nodeType).repository_ != nullptr);

    return entityTypeColumnAdapterHash_.value(nodeType).repository_;
}

void MultiEntityTableModel::resetVariables()
{
    groups_.clear();
    unfind(entities_);
    entities_.clear();
    root_ = nullptr;
    rootIndex_ = QModelIndex();
}

/**
  * @param entityNodeHash [const QHash<AdocNodeType, AdocTreeNodeVector> &]
  * @returns int
  */
int MultiEntityTableModel::sumAcceptableNodes(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash) const
{
    int sum = 0;
    QHash<AdocNodeType, AdocTreeNodeVector>::ConstIterator it = entityNodeHash.constBegin();
    for (; it != entityNodeHash.constEnd(); ++it)
        sum += it.value().size();

    return sum;
}

/**
  * @param entities [const QVector<IEntity *> &]
  */
void MultiEntityTableModel::unfind(const QVector<IEntity *> entities) const
{
    // Unfind each of the entities from the relevant repo. It is important to realize that entities is a mixed array
    QHash<AdocNodeType, QVector<IEntity *> > entitiesByType;
    foreach (IEntity *entity, entities)
        entitiesByType[static_cast<AdocNodeType>(entity->type())] << entity;

    QHash<AdocNodeType, QVector<IEntity *> >::ConstIterator it = entitiesByType.constBegin();
    for (; it != entitiesByType.constEnd(); ++it)
        repositoryForNodeType(it.key())->unfind(it.value());
}



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor for EntityAdapterSpecification
/**
  * @param nModelColumns [int]
  * @param repository [IRepository *]
  * @param columnAdapter [IColumnAdapter *]
  */
MultiEntityTableModel::EntityAdapterSpecification::EntityAdapterSpecification(int nModelColumns, IRepository *repository, IColumnAdapter *columnAdapter)
    : repository_(repository), columnAdapter_(columnAdapter), columnMap_(nModelColumns, IColumnAdapter::kInvalidColumn)
{
}
