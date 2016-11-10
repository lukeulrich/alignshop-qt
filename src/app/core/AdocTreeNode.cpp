/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AdocTreeNode.h"

#include "Entities/TransientTask.h"
#include "../gui/Services/Tasks/ITask.h"
#include "global.h"
#include "macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param sourceEntity [IEntitySPtr &]
  */
AdocTreeNode::AdocTreeNode(const IEntitySPtr &sourceEntity)
    : TreeNode<AdocTreeNode>(),
      loaded_(false),
      entity_(sourceEntity),
      entityId_(0)
{
    ASSERT(sourceEntity);
    nodeType_ = mapNodeType(sourceEntity->type());
}

/**
  * @param nodeType [AdocNodeType]
  * @param label [const QString &]
  * @param entityId [int]
  */
AdocTreeNode::AdocTreeNode(AdocNodeType nodeType, const QString &label, int entityId)
    : TreeNode<AdocTreeNode>(),
      nodeType_(nodeType),
      label_(label),
      loaded_(false),
      entityId_(entityId)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param other [const AdocTreeNode &]
  * @returns bool
  */
bool AdocTreeNode::operator==(const AdocTreeNode &other) const
{
    return nodeType_ == other.nodeType_ &&
            label_ == other.label_ &&
            entityId() == other.entityId();
}

/**
  * @param other [const AdocTreeNode &]
  * @returns bool
  */
bool AdocTreeNode::operator!=(const AdocTreeNode &other) const
{
    return !operator==(other);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool AdocTreeNode::containsNotOverTask() const
{
    for (ConstIterator it = this, end = nextAscendant(); it != end; ++it)
    {
        if (it->nodeType_ != eTransientTaskNode)
            continue;

        ASSERT(it->entity() != nullptr);
        const TransientTaskSPtr &transientTask = boost::shared_static_cast<TransientTask>(it->entity());
        if (transientTask->task() != nullptr && !transientTask->task()->isOver())
            return true;
    }

    return false;
}

/**
  * @returns IEntitySPtr &
  */
IEntitySPtr AdocTreeNode::entity() const
{
    return entity_;
}

/**
  * @returns int
  */
int AdocTreeNode::entityId() const
{
    if (entity_ != nullptr)
        return entity_->id();

    return entityId_;
}

/**
  * newEntity [IEntitySPtr &]
  */
void AdocTreeNode::setEntity(const IEntitySPtr &newEntity)
{
    ASSERT(newEntity);
    entity_ = newEntity;
    entityId_ = 0;
    nodeType_ = mapNodeType(entity_->type());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public static methods
/**
  * @param typeId [int]
  * @returns AdocNodeType
  */
AdocNodeType AdocTreeNode::mapNodeType(int typeId)
{
    if (typeId > eUndefinedNode && typeId < eMaxNode)
        return static_cast<AdocNodeType>(typeId);
    else
        return eUndefinedNode;
}

/**
  * @param nodeType [AdocNodeType]
  * @returns QString
  */
QString AdocTreeNode::textForType(AdocNodeType nodeType)
{
    switch (nodeType)
    {
    case eRootNode:
        return "Root";
    case eGroupNode:
        return "Group";
    case eAminoSeqNode:
        return "AminoSeq";
    case eDnaSeqNode:
        return "DnaSeq";
    case eAminoMsaNode:
        return "AminoMsa";
    case eDnaMsaNode:
        return "DnaMsa";
    case eBlastReportNode:
        return "BlastReport";
    case eTransientTaskNode:
        return "Task";

    default:
        return QString();
    }
}
