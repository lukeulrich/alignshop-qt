/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCNODEDATA_H
#define ADOCNODEDATA_H

#include <QtCore/QString>

#include "Entities/IEntity.h"

#include "enums.h"
#include "macros.h"
#include "global.h"

// We allow the AdocNodeData to maintain a reference to the entity to automatically provide a mechanism for capturing
// the updated id when it is saved (only applies if it is a new entity)
class AdocNodeData
{
public:
    AdocNodeData(AdocNodeType nodeType = eUndefinedNode, const QString &label = QString(), int entityId = 0)
        : nodeType_(nodeType), label_(label), loaded_(false), entityId_(entityId), entity_(nullptr)
    {
    }

    AdocNodeData(IEntity *entity) : entityId_(0), entity_(entity)
    {
        ASSERT(entity != nullptr);
        nodeType_ = mapNodeType(entity->type());
    }

    bool operator==(const AdocNodeData &other) const
    {
        return nodeType_ == other.nodeType_ &&
                label_ == other.label_ &&
                entityId() == other.entityId();
    }

    bool operator!=(const AdocNodeData &other) const
    {
        return !operator==(other);
    }

    int entityId() const
    {
        if (entity_ != nullptr)
            return entity_->id();

        return entityId_;
    }

    IEntity *entity() const
    {
        return entity_;
    }

    static AdocNodeType mapNodeType(int typeId)
    {
        if (typeId > eUndefinedNode && typeId < eMaxNode)
            return static_cast<AdocNodeType>(typeId);
        else
            return eUndefinedNode;
    }

    static QString textForType(AdocNodeType nodeType)
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

    AdocNodeType nodeType_;
    QString label_;
    bool loaded_;       // Optimization flag denoting whether this node has been read from the data source

private:
    int entityId_;
    IEntity *entity_;
};

Q_DECLARE_TYPEINFO(AdocNodeData, Q_PRIMITIVE_TYPE);

#endif // ADOCNODEDATA_H
