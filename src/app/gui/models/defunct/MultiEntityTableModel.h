/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MULTIENTITYTABLEMODEL_H
#define MULTIENTITYTABLEMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QVector>

#include "../../core/global.h"
#include "../../core/enums.h"       // For AdocNodeType

// -------------------------------------------------------------------------------------------------
// Forward declarations
#include "../../core/AdocTreeNode_fwd.h"
class AdocTreeModel;
class IColumnAdapter;
class IEntity;
class IRepository;

/**
  */
class MultiEntityTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        eIdColumn = 0,
        eNameColumn,
        eDescriptionColumn,

        eNumberOfColumns
    };

    struct EntityAdapterSpecification
    {
        IRepository *repository_;
        IColumnAdapter *columnAdapter_;
        QVector<int> columnMap_;

        EntityAdapterSpecification(int nAdaptedColumns = 0, IRepository *repository = nullptr, IColumnAdapter *columnAdapter = nullptr);
    };

    // -------------------------------------------------------------------------------------------------
    // Constructors and destructor
    MultiEntityTableModel(QObject *parent = nullptr);

    // -------------------------------------------------------------------------------------------------
    // Public methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    AdocTreeNode *root() const;
    QModelIndex rootIndex() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    void setAdapterSpecification(int entityType, const EntityAdapterSpecification &entityAdapterSpecification);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void setSourceTreeModel(AdocTreeModel *adocTreeModel);

public slots:
    // -------------------------------------------------------------------------------------------------
    // Public slots
    void setRoot(const QModelIndex &rootIndex);

protected:
    // -------------------------------------------------------------------------------------------------
    // Protected methods
    bool acceptNodeType(int nodeType) const;

private slots:
    // -------------------------------------------------------------------------------------------------
    // Private slots
    void onTreeModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onTreeModelRowsInserted(const QModelIndex &parent, int start, int end);
    void onTreeModelRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void onEntityDataChanged(IEntity *entity, int column);

private:
    // -------------------------------------------------------------------------------------------------
    // Private methods
    IEntity *entityFromIndex(const QModelIndex &index) const;
    QHash<AdocNodeType, AdocTreeNodeVector> extractAcceptableNodes(const AdocTreeNode *parent) const;
    QHash<AdocNodeType, AdocTreeNodeVector> extractAcceptableNodes(const AdocTreeNode *parent, int start, int end) const;
    void findAddNodes(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash);
    bool isGroupIndex(const QModelIndex &index) const;
    IRepository *repositoryForNodeType(AdocNodeType nodeType) const;
    void resetVariables();
    int sumAcceptableNodes(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash) const;
    void unfind(const QVector<IEntity *> entities) const;

    // -------------------------------------------------------------------------------------------------
    // Private members
    AdocTreeModel *adocTreeModel_;
    AdocTreeNode *root_;
    QHash<int, EntityAdapterSpecification> entityTypeColumnAdapterHash_;
    AdocTreeNodeVector groups_;
    QVector<IEntity *> entities_;
    QPersistentModelIndex rootIndex_;
};

Q_DECLARE_TYPEINFO(MultiEntityTableModel::EntityAdapterSpecification, Q_MOVABLE_TYPE);


#endif // MULTIENTITYTABLEMODEL_H
