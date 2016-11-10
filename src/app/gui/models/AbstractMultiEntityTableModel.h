/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTMULTIENTITYTABLEMODEL_H
#define ABSTRACTMULTIENTITYTABLEMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QHash>
#include <QtCore/QQueue>
#include <QtCore/QVector>
#include <QtCore/QScopedPointer>

#include <QtGui/QItemSelection>

#include "../../core/Entities/IEntity.h"
#include "../../core/AdocTreeNode.h"
#include "../../core/enums.h"               // For AdocNodeType
#include "../../core/global.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// Forward declarations
class QUndoStack;
class QTimer;

class AdocTreeModel;
class IColumnAdapter;
class IRepository;
class LoadRequestManager;

/**
  * Models groups and entities.
  */
class AbstractMultiEntityTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    class EntityAdapterSpecification
    {
    public:
        EntityAdapterSpecification(int nModelColumns = 0, IRepository *repository = nullptr, IColumnAdapter *columnAdapter = nullptr);

        void setMapping(int modelColumn, int adapterColumn);
        int mapToModel(int adapterColumn) const;
        int mapToAdapter(int modelColumn) const;

        IRepository *repository_;
        IColumnAdapter *columnAdapter_;

    private:
        QVector<int> modelColumnToAdapterColumn_;
        QHash<int, int> adapterColumnToModelColumn_;
    };

    // -------------------------------------------------------------------------------------------------
    // Public methods
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant dataForEntity(const IEntitySPtr &entity, int column, int role = Qt::DisplayRole) const;                  // For ease of access of column data per entity from multiple places; namely, the data() method and when sorting entities
    bool dynamicSort() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int groupLabelColumn() const = 0;
    bool isGroupIndex(const QModelIndex &index) const;
    QModelIndex mapToTree(int row) const;
    QModelIndex mapToTree(const QModelIndex &tableIndex) const;
    QItemSelection mapSelectionToTree(const QItemSelection &tableSelection) const;
    AdocTreeNode *nodeFromIndex(const QModelIndex &index) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    AdocTreeNode *root() const;
    QModelIndex rootIndex() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    void setAdapterSpecification(int entityType, const EntityAdapterSpecification &entityAdapterSpecification);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void setDynamicSort(bool dynamicSort);
    void setSourceTreeModel(AdocTreeModel *adocTreeModel);
    void setUndoStack(QUndoStack *undoStack);
    void sort();
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    QUndoStack *undoStack() const;

    // ISSUE? Should this really be a public method? Originally it was protected
    IEntitySPtr entityFromIndex(const QModelIndex &index) const;

    // Drag and drop
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QMimeData *mimeData(const QModelIndexList &indices) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;


Q_SIGNALS:
    void rootChanged(const QModelIndex &newRootIndex);


public Q_SLOTS:
    // -------------------------------------------------------------------------------------------------
    // Public slots
    virtual void refreshEntityIds();                        // Emits a dataChanged signal for the system id column
    void setRoot(const QModelIndex &rootIndex);


protected:
    // -------------------------------------------------------------------------------------------------
    // Protected methods
    AbstractMultiEntityTableModel(QObject *parent = nullptr);
    ~AbstractMultiEntityTableModel();
    bool acceptNodeType(int nodeType) const;


private slots:
    // -------------------------------------------------------------------------------------------------
    // Private slots
    void onTreeModelReset();
    void onTreeModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onTreeModelRowsInserted(const QModelIndex &parent, int start, int end);
    void onTreeModelRowsAboutToBeMoved(const QModelIndex &srcParentIndex, int srcStart, int srcEnd, const QModelIndex &dstParentIndex, int dstRow);
    void onTreeModelRowsMoved(const QModelIndex &srcParentIndex, int srcStart, int srcEnd, const QModelIndex &dstParentIndex, int dstRow);
    void onTreeModelRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void onTreeModelTransientEntityReplaced(const QModelIndex &treeIndex, const IEntitySPtr &newEntity, const IEntitySPtr &oldEntity);
    void onEntityDataChanged(const IEntitySPtr &entity, int column);

    void processLoadRequest();

private:
    // -------------------------------------------------------------------------------------------------
    // Private structs
    // Helper struct for updating persistent indices when the order changes
    struct PersistentIndexData
    {
        PersistentIndexData() : column_(0), isEntity_(false), node_(nullptr)
        {}

        PersistentIndexData(int column, const IEntitySPtr &entity)
            : column_(column), isEntity_(true), entity_(entity), node_(nullptr)
        {}

        PersistentIndexData(int column, AdocTreeNode *node)
            : column_(column), isEntity_(false), node_(node)
        {}

        AdocTreeNode *node() const
        {
            return node_;
        }

        IEntitySPtr entity() const
        {
            return entity_;
        }

        int column_;
        bool isEntity_;
        IEntitySPtr entity_;
        AdocTreeNode *node_;
    };

    // -------------------------------------------------------------------------------------------------
    // Private methods
    IEntitySPtr entityFromRow(int row) const;
    QHash<AdocNodeType, AdocTreeNodeVector> extractAcceptableNodes(const AdocTreeNode *parent) const;
    QHash<AdocNodeType, AdocTreeNodeVector> extractAcceptableNodes(const AdocTreeNode *parent, int start, int end) const;
    void findAddNodes(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash);
    void findAddNodes(AdocNodeType adocNodeType, const AdocTreeNodeVector &adocTreeNodeVector, int start, int end);
    QModelIndex indexFromEntity(const IEntitySPtr &entity, int column = 0) const;
    QModelIndex indexFromGroupNode(AdocTreeNode *groupNode, int column = 0) const;
    int mapEntityColumn(int entityType, int entityColumn) const;            // Returns the model column corresponding to entityColumn for entityType or constants::kInvalidColumn if not found
    int mapEntityRowToModelRow(int entityRow) const;
    QModelIndex mapFromTree(const QModelIndex &index) const;
    QModelIndexList mapToModelIndices(const QVector<PersistentIndexData> &persistentIndexData) const;
    QVector<PersistentIndexData> mapToPersistentIndexData(const QModelIndexList &modelIndexList) const;
    IRepository *repositoryForNodeType(AdocNodeType nodeType) const;
    void resetVariables();
    int sumAcceptableNodes(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash) const;

    // -------------------------------------------------------------------------------------------------
    // Private members
    AdocTreeModel *adocTreeModel_;
    AdocTreeNode *root_;
    QHash<int, EntityAdapterSpecification> entityTypeAdapterHash_;    // entity type -> adapter specification
    AdocTreeNodeVector groups_;
    QVector<IEntitySPtr> entities_;
    QPersistentModelIndex rootIndex_;
    QHash<IEntitySPtr, AdocTreeNode *> entityNodeHash_;                 // Necessary for mapping the nodes back to their model indices in AdocTreeModel
    QUndoStack *undoStack_;

    // All things sorting related
    struct SortParams
    {
        bool dynamic_;
        int column_;
        Qt::SortOrder order_;
    };
    SortParams sortParams_;

    QTimer *loadTimer_;
    QScopedPointer<LoadRequestManager> loadRequestManager_;
    struct LoadingContainer
    {
        AdocTreeNodeVector groups_;
        QVector<IEntitySPtr> entities_;
        QHash<IEntitySPtr, AdocTreeNode *> entityNodeHash_;

        void clear()
        {
            groups_.clear();
            entities_.clear();
            entityNodeHash_.clear();
        }
    };
    LoadingContainer loadingContainer_;
};

Q_DECLARE_TYPEINFO(AbstractMultiEntityTableModel::EntityAdapterSpecification, Q_MOVABLE_TYPE);


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Helper load request class
struct LoadRequestChunk
{
    LoadRequestChunk(AdocNodeType nodeType = eUndefinedNode,
                     const AdocTreeNodeVector &adocNodeVector = AdocTreeNodeVector(),
                     int start = 0,
                     int end = -1)
        : nodeType_(nodeType), nodeVector_(adocNodeVector), start_(start), end_(end)
    {
    }

    AdocNodeType nodeType_;             // To get the repository
    AdocTreeNodeVector nodeVector_;     // Nodes to retrieve
    int start_;                         // Range of vector to retrieve
    int end_;
};


/*
   -- Should I ever want to track per-node loading in batch, the following code demonstrates the necessary changes to
      make this happen

int subTotal = 0;
while (subTotal != partialRequestSize_ && !isDone())
{
    int remainingRequestSpace = partialRequestSize_ - subTotal;
    int numberOfNodesInCurrentType = entityNodeHash_.value(currentType_).size();
    int numberofNodesNotYetRequested = numberOfNodesInCurrentType - offset_;
    int amountToFetchOfCurrentType = 0;
    int unloadedEntitiesOfCurrentType = 0;

    const AdocTreeNodeVector &adocTreeNodeVector = entityNodeHash_.value(currentType_);
    while (unloadedEntitiesOfCurrentType < remainingRequestSpace &&
           amountToFetchOfCurrentType < numberofNodesNotYetRequested)
    {
        ++amountToFetchOfCurrentType;
        if (adocTreeNodeVector.at(offset_ + amountToFetchOfCurrentType)->data_.entity_ == nullptr)
            ++unloadedEntitiesOfCurrentType;
    }

    plr.loadRequestChunks_ << LoadRequestChunk(currentType_, adocTreeNodeVector, offset_, offset_ + amountToFetchOfCurrentType - 1);
    subTotal += unloadedEntitiesOfCurrentType;
    offset_ += amountToFetchOfCurrentType;
    if (offset_ == numberOfNodesInCurrentType)
    {
        entityNodeHash_.remove(currentType_);
        currentType_ = getUnfinishedType();
        offset_ = 0;
    }
}
*/

// Required by MAC to compile with the clang++ toolchain
typedef QHash<AdocNodeType, AdocTreeNodeVector> TypeNodeVectorHash;
class LoadRequestManager
{
public:
    LoadRequestManager(const TypeNodeVectorHash &entityNodeHash = TypeNodeVectorHash(),
                       int partialRequestSize = 1000)
        : offset_(0),
          partialRequestSize_(partialRequestSize)
    {
        if (entityNodeHash.isEmpty() == false)
            entityNodeHashQueue_ << entityNodeHash;
        currentType_ = getUnfinishedType();
    }

    bool isDone() const
    {
        return entityNodeHashQueue_.isEmpty();
    }

    void clear()
    {
        entityNodeHashQueue_.clear();
    }

    void addBatch(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash)
    {
        if (entityNodeHash.isEmpty() == false)
        {
            entityNodeHashQueue_.enqueue(entityNodeHash);

            // Usually this method is called in response to TreeModelRowsInserted and thus the loadRequestManager may
            // already be loading other rows for the same parent. For the first case, however, the queue will be empty
            // and have an undefined current type.
            if (entityNodeHashQueue_.size() == 1)
                currentType_ = getUnfinishedType();
        }
    }

    QVector<LoadRequestChunk> nextBatch()
    {
        ASSERT(isDone() == false);

        QVector<LoadRequestChunk> loadRequestChunkVector;

        int subTotal = 0;
        while (subTotal != partialRequestSize_ && !isDone())
        {
            QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash_ = entityNodeHashQueue_.head();

            int remainingRequestSpace = partialRequestSize_ - subTotal;
            int numberOfNodesInCurrentType = entityNodeHash_.value(currentType_).size();
            int numberofNodesNotYetRequested = numberOfNodesInCurrentType - offset_;
            int amountToFetchOfCurrentType = qMin(remainingRequestSpace, numberofNodesNotYetRequested);

            loadRequestChunkVector << LoadRequestChunk(currentType_, entityNodeHash_.value(currentType_), offset_, offset_ + amountToFetchOfCurrentType - 1);
            subTotal += amountToFetchOfCurrentType;
            offset_ += amountToFetchOfCurrentType;
            if (offset_ == numberOfNodesInCurrentType)
            {
                entityNodeHash_.remove(currentType_);
                if (entityNodeHash_.isEmpty())
                    entityNodeHashQueue_.dequeue();
                currentType_ = getUnfinishedType();
                offset_ = 0;
            }
        }
        return loadRequestChunkVector;
    }

private:
    AdocNodeType getUnfinishedType() const
    {
        if (!isDone())
            return entityNodeHashQueue_.head().keys().first();

        return eUndefinedNode;
    }

    QQueue<QHash<AdocNodeType, AdocTreeNodeVector> > entityNodeHashQueue_;
    int offset_;
    AdocNodeType currentType_;
    int partialRequestSize_;
};


#endif // ABSTRACTMULTIENTITYTABLEMODEL_H
