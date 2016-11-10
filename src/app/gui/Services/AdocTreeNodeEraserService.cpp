/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AdocTreeNodeEraserService.h"

#include "../models/AdocTreeModel.h"
#include "../../core/Entities/TransientTask.h"
#include "../../core/Adoc.h"

#include "../../core/global.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adoc [Adoc *]
  * @param adocTreeModel [AdocTreeModel *]
  * @param parent [QObject *]
  */
AdocTreeNodeEraserService::AdocTreeNodeEraserService(Adoc *adoc, AdocTreeModel *adocTreeModel, QObject *parent) :
    QObject(parent), adoc_(adoc), adocTreeModel_(adocTreeModel)
{
    if (adoc_ != nullptr &&
        adocTreeModel_ != nullptr)
    {
        connect(adocTreeModel_,
                SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
                SLOT(onTreeRowsAboutToBeRemoved(QModelIndex, int, int)));

        connect(adocTreeModel_,
                SIGNAL(nodesAboutToBeAdded(AdocTreeNodeVector)),
                SLOT(onTreeNodesAboutToBeAdded(AdocTreeNodeVector)));

        connect(adocTreeModel_,
                SIGNAL(transientEntityReplaced(QModelIndex,IEntitySPtr,IEntitySPtr)),
                SLOT(onTransientEntityReplaced(QModelIndex,IEntitySPtr,IEntitySPtr)));
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Adoc *
  */
Adoc *AdocTreeNodeEraserService::adoc() const
{
    return adoc_;
}

/**
  * @returns AdocTreeModel *
  */
AdocTreeModel *AdocTreeNodeEraserService::adocTreeModel() const
{
    return adocTreeModel_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void AdocTreeNodeEraserService::onTreeRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    ASSERT(adoc_ != nullptr);
    ASSERT(adocTreeModel_ != nullptr);
    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(parent);
    if (parentNode == nullptr)
        return;

    ASSERT(start >= 0 && start <= end);
    ASSERT(end < parentNode->childCount());

    AdocTreeNodeVector entityNodes = extractEntityNodes(parentNode->childrenBetween(start, end));
    if (entityNodes.isEmpty())
        return;

    QHash<EntityType, QVector<int> > entityIdVectorHash = groupIdsByEntityType(entityNodes);
    QHash<EntityType, QVector<int> >::ConstIterator it = entityIdVectorHash.constBegin();
    while (it != entityIdVectorHash.constEnd())
    {
        adoc_->repository(it.key())->erase(it.value());
        ++it;
    }
}

/**
  * May be called unnecessarily. For instance, when the user imports some sequences they will not need to be "unerased"
  * however, it does no harm besides eating a few extra cycles to request that the relevant repos unerase these ids.
  *
  * @param adocTreeNodeVector [const AdocTreeNodeVector &]
  */
void AdocTreeNodeEraserService::onTreeNodesAboutToBeAdded(const AdocTreeNodeVector &adocTreeNodeVector)
{
    AdocTreeNodeVector entityNodes = extractEntityNodes(adocTreeNodeVector);
    if (entityNodes.isEmpty())
        return;

    QHash<EntityType, QVector<int> > entityIdVectorHash = groupIdsByEntityType(entityNodes);
    QHash<EntityType, QVector<int> >::ConstIterator it = entityIdVectorHash.constBegin();
    while (it != entityIdVectorHash.constEnd())
    {
        ASSERT_X(adoc_->repository(it.key()) != nullptr, "Every entity must have a corresponding repository");
        adoc_->repository(it.key())->unerase(it.value());
        ++it;
    }
}

/**
  * Remove the old entity from the transient repository.
  *
  * @param index [const QModelIndex &]
  * @param newEntity [IEntitySPtr &]
  * @param oldEntity [IEntitySPtr &]
  */
void AdocTreeNodeEraserService::onTransientEntityReplaced(const QModelIndex & /* index */, const IEntitySPtr & /* newEntity */, const IEntitySPtr &oldEntity)
{
    ASSERT(adoc_ != nullptr);
    ASSERT(boost::shared_dynamic_cast<TransientTask>(oldEntity) != 0);

    adoc_->transientTaskRepository()->eraseGeneric(oldEntity);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Recursive method for extracting entity nodes from all members and descendants of the nodes vector. An entity node
  * is recognized via a non-zero id.
  *
  * @param nodes [const QVector<AdocTreeNode *> &]
  * @returns QVector<AdocTreeNode *>
  */
AdocTreeNodeVector AdocTreeNodeEraserService::extractEntityNodes(const AdocTreeNodeVector &nodes) const
{
    AdocTreeNodeVector entityNodes;
    for (int i=0, z= nodes.size(); i<z; ++i)
    {
        AdocTreeNode *node = nodes.at(i);
        if (node->entityId() != 0)
            entityNodes << node;

        // Iteratively check all children! :) Probably should ask user for confirmation
        if (node->childCount() > 0)
            entityNodes << extractEntityNodes(node->children());
    }

    return entityNodes;
}

/**
  * @param nodes [const QVector<AdocTreeNode *> &]
  * @returns QHash<EntityType, QVector<int> >
  */
QHash<EntityType, QVector<int> > AdocTreeNodeEraserService::groupIdsByEntityType(const AdocTreeNodeVector &nodes) const
{
    QHash<EntityType, QVector<int> > entityIdVectorHash;
    for (int i=0, z=nodes.size(); i<z; ++i)
    {
        AdocTreeNode *node = nodes.at(i);
        ASSERT(node->entityId() != 0);
        EntityType entityType = static_cast<EntityType>(node->nodeType_);
        entityIdVectorHash[entityType] << node->entityId();
    }

    return entityIdVectorHash;
}

/**
  * @param nodes [const QVector<AdocTreeNode *> &]
  * @returns QHash<EntityType, QVector<int> >
  */
QHash<EntityType, QVector<IEntitySPtr> > AdocTreeNodeEraserService::groupEntitiesByEntityType(const AdocTreeNodeVector &nodes) const
{
    QHash<EntityType, QVector<IEntitySPtr> > entityVectorHash;
    for (int i=0, z=nodes.size(); i<z; ++i)
    {
        AdocTreeNode *node = nodes.at(i);
        ASSERT(node->entityId() != 0);
        if (node->entity() != nullptr)
        {
            EntityType entityType = static_cast<EntityType>(node->nodeType_);
            entityVectorHash[entityType] << node->entity();
        }
    }

    return entityVectorHash;
}
