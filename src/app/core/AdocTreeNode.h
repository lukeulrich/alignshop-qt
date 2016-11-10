/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREENODE_H
#define ADOCTREENODE_H

#include <QtCore/QString>

#include "TreeNode.h"
#include "enums.h"

#include "Entities/IEntity.h"

class AdocTreeNode : public TreeNode<AdocTreeNode>
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    AdocTreeNode(const IEntitySPtr &sourceEntity);
    AdocTreeNode(AdocNodeType nodeType = eUndefinedNode, const QString &label = QString(), int entityId = 0);


    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const AdocTreeNode &other) const;
    bool operator!=(const AdocTreeNode &other) const;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool containsNotOverTask() const;
    IEntitySPtr entity() const;
    int entityId() const;
    void setEntity(const IEntitySPtr &newEntity);


    // ------------------------------------------------------------------------------------------------
    // Static public methods
    static AdocNodeType mapNodeType(int typeId);
    static QString textForType(AdocNodeType nodeType);


    // ------------------------------------------------------------------------------------------------
    // Public members
    AdocNodeType nodeType_;
    QString label_;
    bool loaded_;       // Optimization flag denoting whether this node has been read from the data source


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    IEntitySPtr entity_;
    int entityId_;
};

Q_DECLARE_TYPEINFO(AdocTreeNode, Q_MOVABLE_TYPE);

typedef QVector<AdocTreeNode *> AdocTreeNodeVector;

#endif // ADOCTREENODE_H
