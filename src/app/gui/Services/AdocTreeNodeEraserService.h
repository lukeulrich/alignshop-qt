/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREENODEERASERSERVICE_H
#define ADOCTREENODEERASERSERVICE_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QVector>

#include "../../core/AdocTreeNode.h"
#include "../../core/Entities/IEntity.h"
#include "../../core/enums.h"

class QModelIndex;
class Adoc;
class AdocTreeModel;
class IEntity;

/**
  * Synchronizes an AdocTreeModel with a set of repositories. When rows are removed from the tree, the corresponding
  * entries are then erased from the repository. Similarly, when rows are restored, they are unerased from the
  * repository.
  */
class AdocTreeNodeEraserService : public QObject
{
    Q_OBJECT

public:
    AdocTreeNodeEraserService(Adoc *adoc, AdocTreeModel *adocTreeModel, QObject *parent  = 0);

    Adoc *adoc() const;
    AdocTreeModel *adocTreeModel() const;

private slots:
    void onTreeRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void onTreeNodesAboutToBeAdded(const AdocTreeNodeVector &adocTreeNodeVector);
    void onTransientEntityReplaced(const QModelIndex &index, const IEntitySPtr &newEntity, const IEntitySPtr &oldEntity);

private:
    AdocTreeNodeVector extractEntityNodes(const AdocTreeNodeVector &nodes) const;
    QHash<EntityType, QVector<int> > groupIdsByEntityType(const AdocTreeNodeVector &nodes) const;
    QHash<EntityType, QVector<IEntitySPtr> > groupEntitiesByEntityType(const AdocTreeNodeVector &nodes) const;

    Adoc *adoc_;
    AdocTreeModel *adocTreeModel_;
};

#endif // ADOCTREENODEERASERSERVICE_H
