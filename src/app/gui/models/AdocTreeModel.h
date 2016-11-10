/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREEMODEL_H
#define ADOCTREEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMimeData>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QVector>

#include "AbstractBaseTreeModel.h"
#include "../../core/AdocTreeNode.h"

// -------------------------------------------------------------------------------------------------
// Forward declarations
class QUndoStack;

/**
  */
class AdocTreeModel : public AbstractBaseTreeModel<AdocTreeNode>
{
    Q_OBJECT

public:
    explicit AdocTreeModel(QObject *parent = 0);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    // Drag and drop
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QMimeData *mimeData(const QModelIndexList &indices) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;

    AdocTreeNode *root() const;
    void setRoot(AdocTreeNode *root);

    bool appendRow(AdocTreeNode *node, const QModelIndex &parent);
    bool appendRows(const AdocTreeNodeVector &nodes, const QModelIndex &parent);
    void cutRows(const QModelIndexList &indices);
    bool hasCutRows() const;
    bool moveRows(int srcRow, int count, const QModelIndex &srcParent, const QModelIndex &dstParent);
    QModelIndex newGroup(const QString &groupName, const QModelIndex &parent);
    void paste(const QModelIndex &index);
    bool removeRows(const QModelIndexList &indices);
    bool removeRows(int row, int count, const QModelIndex &parent);

    // Now all modifications can use the undo stack if defined :)
    void setUndoStack(QUndoStack *undoStack);
    QUndoStack *undoStack() const;


public Q_SLOTS:
    void clearCutCopyRows();
    bool replaceTransientEntity(const QModelIndex &index, IEntitySPtr &entity);


Q_SIGNALS:
    void nodesAboutToBeAdded(const AdocTreeNodeVector &adocTreeNodeVector);
    void nodesAboutToBeDestroyed(const AdocTreeNodeVector &adocTreeNodeVector);     // Called by command classes whenever the nodes in adocTreeNodeVector are about to be destroyed
    void transientEntityReplaced(const QModelIndex &index, const IEntitySPtr &newEntity, const IEntitySPtr &oldEntity);


protected:
    bool setDataPrivate(const QModelIndex &index, const QVariant &value);


private:
    int addRows(const AdocTreeNodeVector &nodes, AdocTreeNode *parentNode);
    bool destroyRows(int row, int count, const QModelIndex &parent);
    int moveRows(int srcRow, int count, AdocTreeNode *srcParentNode, AdocTreeNode *dstParentNode);
    bool moveRows(const QModelIndexList &modelIndexList, AdocTreeNode *dstParentNode);
    bool moveRows(const QVector<QPersistentModelIndex> &persistentModelIndexList, AdocTreeNode *dstParentNode);
    AdocTreeNodeVector takeRows(int row, int count, const QModelIndex &parent);

    QUndoStack *undoStack_;

    // Cut-Copy-Paste control
    QVector<QPersistentModelIndex> cutOrCopyIndices_;

    friend class InsertAdocTreeNodesCommand;
    friend class MoveAdocTreeNodesCommand;
    friend class RemoveAdocTreeNodesCommand;
    friend class SetGroupLabelCommand;

    friend class TaskAdocConnector;
};




class AdocTreeMimeData : public QMimeData
{
public:
    AdocTreeMimeData()
    {
    }

    AdocTreeMimeData(const QModelIndexList &indices) : indices_(indices)
    {
    }

    QModelIndexList indices() const
    {
        return indices_;
    }

private:
    QModelIndexList indices_;
};

#endif // ADOCTREEMODEL_H
