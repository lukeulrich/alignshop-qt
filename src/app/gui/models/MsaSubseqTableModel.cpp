/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QMimeData>
#include <QtCore/QStringList>

#include "MsaSubseqTableModel.h"
#include "ColumnAdapters/IColumnAdapter.h"
#include "../Commands/Msa/SortMsaCommand.h"
#include "../Commands/Msa/MoveRowsCommand.h"
#include "../../core/Entities/AbstractSeq.h"
#include "../../core/ObservableMsa.h"
#include "../../core/util/QVariantLessGreaterThan.h"
#include "../../core/macros.h"
#include "../../core/misc.h"
#include "../gui_misc.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private structures
class RowMimeData : public QMimeData
{
public:
    RowMimeData()
    {}

    RowMimeData(const QModelIndexList &indices) :
        rows_(::reduceToUniqueRows(indices))
    {
        for (int i=0, z=rows_.size(); i<z; ++i)
            ++rows_[i];
    }

    QVector<int> rows() const
    {
        return rows_;
    }

private:
    QVector<int> rows_;
};


class SubseqLessThanPrivate : public ISubseqLessThan
{
public:
    SubseqLessThanPrivate(MsaSubseqTableModel *model, int column) : model_(model), column_(column)
    {
        ASSERT(model != nullptr);
    }

    bool lessThan(const Subseq *a, const Subseq *b) const
    {
        return QVariantLessThan(model_->data(a, column_), model_->data(b, column_));
    }

private:
    MsaSubseqTableModel *model_;
    int column_;
};

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
MsaSubseqTableModel::MsaSubseqTableModel(QObject *parent)
    : QAbstractTableModel(parent), msa_(nullptr), columnAdapter_(nullptr), undoStack_(nullptr)
{
}

/**
  * @param msa [ObservableMsa *]
  * @param columnAdapter [IColumnAdapter *]
  * @param parent [QObject *]
  */
MsaSubseqTableModel::MsaSubseqTableModel(ObservableMsa *msa, IColumnAdapter *columnAdapter, QObject *parent)
    : QAbstractTableModel(parent), msa_(nullptr), columnAdapter_(nullptr), undoStack_(nullptr)
{
    setMsa(msa, columnAdapter);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int MsaSubseqTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() == false && msa_ != nullptr)
        return msa_->rowCount();

    return 0;
}

/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int MsaSubseqTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() == false && columnAdapter_ != nullptr)
        return eNumberOfSelfColumns + columnAdapter_->columnCount();

    return eNumberOfSelfColumns;
}

/**
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant MsaSubseqTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || msa_ == nullptr)
        return QVariant();

    ASSERT(index.row() >= 0 && index.row() < msa_->rowCount());

    // For now - disable all decorations...
    if (role == Qt::DecorationRole)
        return QVariant();

    return data(msa_->at(index.row() + 1), index.column(), role);
}

/**
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags MsaSubseqTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.isValid() == false)
    {
        flags |= Qt::ItemIsDropEnabled;
        return flags;
    }

    ASSERT(index.column() >= 0 && index.column() < columnCount());
    if (index.column() < eNumberOfSelfColumns)
        return flags | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    if (columnAdapter_ != nullptr)
        flags = columnAdapter_->flags(index.column() - eNumberOfSelfColumns);
    flags |= Qt::ItemIsDragEnabled;

    return flags;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant MsaSubseqTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return section + 1;

    // Else horizontal orientation
    switch (section)
    {
    case eStartColumn:
        return "Start";
    case eStopColumn:
        return "Stop";

    default:
        break;
    }

    if (columnAdapter_ != nullptr)
        return columnAdapter_->headerData(section - eNumberOfSelfColumns);

    return QVariant();
}

/**
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  * @returns bool
  */
bool MsaSubseqTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || msa_ == nullptr || role != Qt::EditRole)
        return false;

    ASSERT(index.row() >= 0 && index.row() < msa_->rowCount());

    if (index.column() > eNumberOfSelfColumns && columnAdapter_ != nullptr)
        return columnAdapter_->setData(msa_->at(index.row() + 1)->seqEntity_, index.column() - eNumberOfSelfColumns, value);

    return false;
}

/**
  * @param column [int]
  * @param order [Qt::SortOrder]
  */
void MsaSubseqTableModel::sort(int column, Qt::SortOrder order)
{
    ASSERT(msa_ != nullptr);

    if (undoStack_ != nullptr)
        undoStack_->push(new SortMsaCommand(msa_, new SubseqLessThanPrivate(this, column), order));
    else
        msa_->sort(SubseqLessThanPrivate(this, column), order);
}

bool MsaSubseqTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int /* column */, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (action != Qt::MoveAction)
        return false;

    const RowMimeData *rowMimeData = static_cast<const RowMimeData *>(data);
    if (!rowMimeData->hasFormat("application/alignshop.rows"))
        return false;

    int dropRow;
    if (row != -1)
        dropRow = row + 1;
    else if (parent.isValid())
        dropRow = parent.row() + 1;
    else
        dropRow = rowCount() + 1;

    QVector<ClosedIntRange> ranges = ::convertIntVectorToClosedIntRanges(rowMimeData->rows());
    if (ranges.size() == 1)
    {
        ClosedIntRange range = ranges.first();
        if (dropRow == range.begin_)
            return false;

        int destRow;
        if (dropRow < range.begin_)
            destRow = dropRow;
        else if (dropRow <= range.end_ + 1)
            return false;
        else
            destRow = dropRow - range.length();

        MoveRowsCommand *moveCommand = new MoveRowsCommand(msa_, ranges.first(), destRow);
        undoStack_->push(moveCommand);
    }
    else
    {
        undoStack_->beginMacro("Move discontiguous rows");
        int x = dropRow;
        for (int i=0, z=ranges.size(); i<z; ++i)
        {
            const ClosedIntRange &range = ranges.at(i);
            if (x < range.begin_)
                undoStack_->push(new MoveRowsCommand(msa_, range, x));
            else if (x > range.end_ + 1)
            {
                undoStack_->push(new MoveRowsCommand(msa_, range, x - range.length()));

                for (int j=i+1; j<z; ++j)
                {
                    if (ranges.at(j).begin_ >= x)
                        break;

                    ranges[j] -= range.length();
                }

                continue;
            }
            else if (x == range.end_ + 1)
                continue;
            x += range.length();
        }
        undoStack_->endMacro();
    }

    // If we return true, then the model will call removeRows, we don't want that, since the real model is the MSA
    return false;
}

QMimeData *MsaSubseqTableModel::mimeData(const QModelIndexList &indices) const
{
    QMimeData *mimeData = new RowMimeData(indices);
    mimeData->setData("application/alignshop.rows", QByteArray());
    return mimeData;
}

QStringList MsaSubseqTableModel::mimeTypes() const
{
    QStringList types;
    types << "application/alignshop.rows";
    return types;
}

Qt::DropActions MsaSubseqTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

/**
  * @returns IColumnAdapter *
  */
IColumnAdapter *MsaSubseqTableModel::columnAdapter() const
{
    return columnAdapter_;
}

/**
  * @param column [int]
  */
int MsaSubseqTableModel::mapFromColumnAdapter(int column) const
{
    if (columnAdapter_ == nullptr)
        return -1;

#ifdef QT_DEBUG
    if (column < 0 || column >= columnAdapter_->columnCount())
        qWarning("[%s] Column %d is out of range\n", Q_FUNC_INFO, column);
#endif

    if (column >= 0 && column < columnAdapter_->columnCount())
        return column + eNumberOfSelfColumns;

    return -1;
}

/**
  * @returns ObservableMsa *
  */
ObservableMsa *MsaSubseqTableModel::msa() const
{
    return msa_;
}

/**
  * @param msa [ObservableMsa *]
  * @param columnAdapter [IColumnAdapter *]
  */
void MsaSubseqTableModel::setMsa(ObservableMsa *msa, IColumnAdapter *columnAdapter)
{
    beginResetModel();
    if (msa_)
    {
        disconnect(msa_, SIGNAL(msaAboutToBeReset()), this, SIGNAL(modelAboutToBeReset()));
        disconnect(msa_, SIGNAL(msaReset()), this, SIGNAL(modelReset()));
        disconnect(msa_, SIGNAL(rowsAboutToBeInserted(ClosedIntRange)), this, SLOT(onMsaRowsAboutToBeInserted(ClosedIntRange)));
        disconnect(msa_, SIGNAL(rowsAboutToBeMoved(ClosedIntRange,int)), this, SLOT(onMsaRowsAboutToBeMoved(ClosedIntRange,int)));
        disconnect(msa_, SIGNAL(rowsAboutToBeRemoved(ClosedIntRange)), this, SLOT(onMsaRowsAboutToBeRemoved(ClosedIntRange)));
        disconnect(msa_, SIGNAL(rowsAboutToBeSorted()), this, SIGNAL(layoutAboutToBeChanged()));
        disconnect(msa_, SIGNAL(rowsAboutToBeSwapped(int,int)), this, SIGNAL(layoutAboutToBeChanged()));
        disconnect(msa_, SIGNAL(rowsInserted(ClosedIntRange)), this, SLOT(onMsaRowsInserted(ClosedIntRange)));
        disconnect(msa_, SIGNAL(rowsMoved(ClosedIntRange,int)), this, SLOT(onMsaRowsMoved(ClosedIntRange,int)));
        disconnect(msa_, SIGNAL(rowsRemoved(ClosedIntRange)), this, SLOT(onMsaRowsRemoved(ClosedIntRange)));
        disconnect(msa_, SIGNAL(rowsSorted()), this, SIGNAL(layoutChanged()));
        disconnect(msa_, SIGNAL(rowsSwapped(int,int)), this, SIGNAL(layoutChanged()));
        disconnect(msa_, SIGNAL(subseqsChanged(SubseqChangePodVector)), this, SLOT(onMsaSubseqsChanged(SubseqChangePodVector)));

        if (columnAdapter_ != nullptr)
            disconnect(columnAdapter_, SIGNAL(dataChanged(IEntitySPtr,int)), this, SLOT(onSubseqEntityDataChanged(IEntitySPtr,int)));
    }

    msa_ = msa;
    columnAdapter_ = columnAdapter;

    if (msa_)
    {
        connect(msa_, SIGNAL(msaAboutToBeReset()), SIGNAL(modelAboutToBeReset()));
        connect(msa_, SIGNAL(msaReset()), SIGNAL(modelReset()));
        connect(msa_, SIGNAL(rowsAboutToBeInserted(ClosedIntRange)), SLOT(onMsaRowsAboutToBeInserted(ClosedIntRange)));
        connect(msa_, SIGNAL(rowsAboutToBeMoved(ClosedIntRange,int)), SLOT(onMsaRowsAboutToBeMoved(ClosedIntRange,int)));
        connect(msa_, SIGNAL(rowsAboutToBeRemoved(ClosedIntRange)), SLOT(onMsaRowsAboutToBeRemoved(ClosedIntRange)));
        connect(msa_, SIGNAL(rowsAboutToBeSorted()), SIGNAL(layoutAboutToBeChanged()));
        connect(msa_, SIGNAL(rowsAboutToBeSwapped(int,int)), SIGNAL(layoutAboutToBeChanged()));
        connect(msa_, SIGNAL(rowsInserted(ClosedIntRange)), SLOT(onMsaRowsInserted(ClosedIntRange)));
        connect(msa_, SIGNAL(rowsMoved(ClosedIntRange,int)), SLOT(onMsaRowsMoved(ClosedIntRange,int)));
        connect(msa_, SIGNAL(rowsRemoved(ClosedIntRange)), SLOT(onMsaRowsRemoved(ClosedIntRange)));
        connect(msa_, SIGNAL(rowsSorted()), SIGNAL(layoutChanged()));
        connect(msa_, SIGNAL(rowsSwapped(int,int)), SIGNAL(layoutChanged()));
        connect(msa_, SIGNAL(subseqsChanged(SubseqChangePodVector)), SLOT(onMsaSubseqsChanged(SubseqChangePodVector)));

        if (columnAdapter_ != nullptr)
            connect(columnAdapter_, SIGNAL(dataChanged(IEntitySPtr,int)), SLOT(onSubseqEntityDataChanged(IEntitySPtr,int)));
    }
    endResetModel();
}

/**
  * @param undoStack [QUndoStack *]
  */
void MsaSubseqTableModel::setUndoStack(QUndoStack *undoStack)
{
    undoStack_ = undoStack;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param rows [const ClosedIntRange &]
  */
void MsaSubseqTableModel::onMsaRowsAboutToBeInserted(const ClosedIntRange &rows)
{
    beginInsertRows(QModelIndex(), rows.begin_ - 1, rows.end_ - 1);
}

/**
  * @param rows [const ClosedIntRange &]
  * @param finalRow [int]
  */
void MsaSubseqTableModel::onMsaRowsAboutToBeMoved(const ClosedIntRange &rows, int finalRow)
{
    ASSERT(finalRow != rows.begin_);

    int modelDestRow = (finalRow < rows.begin_) ? finalRow - 1
                                                : finalRow + rows.length() - 1;
    beginMoveRows(QModelIndex(), rows.begin_ - 1, rows.end_ - 1, QModelIndex(), modelDestRow);
}

/**
  * @param rows [const ClosedIntRange &]
  */
void MsaSubseqTableModel::onMsaRowsAboutToBeRemoved(const ClosedIntRange &rows)
{
    beginRemoveRows(QModelIndex(), rows.begin_ - 1, rows.end_ - 1);
}

/**
  * @param rows [const ClosedIntRange &]
  */
void MsaSubseqTableModel::onMsaRowsInserted(const ClosedIntRange & /* rows */)
{
    endInsertRows();
}

/**
  * @param rows [const ClosedIntRange &]
  * @param finalRow [int]
  */
void MsaSubseqTableModel::onMsaRowsMoved(const ClosedIntRange & /* rows */, int /* finalRow */)
{
    endMoveRows();
}

/**
  * @param rows [const ClosedIntRange &]
  */
void MsaSubseqTableModel::onMsaRowsRemoved(const ClosedIntRange & /* rows */)
{
    endRemoveRows();
}

/**
  * @param subseqChangePods [const SubseqChangePodVector &]
  */
void MsaSubseqTableModel::onMsaSubseqsChanged(const SubseqChangePodVector &subseqChangePods)
{
    foreach (const SubseqChangePod &pod, subseqChangePods)
    {
        switch (pod.operation_)
        {
        case SubseqChangePod::eExtendLeft:
        case SubseqChangePod::eTrimLeft:
            {
                QModelIndex i = index(pod.row_ - 1, eStartColumn);
                emit dataChanged(i, i);
            }
            break;
        case SubseqChangePod::eExtendRight:
        case SubseqChangePod::eTrimRight:
            {
                QModelIndex i = index(pod.row_ - 1, eStopColumn);
                emit dataChanged(i, i);
            }
            break;

        default:
            break;
        }
    }
}

/**
  * Most useful when responding to changes originating from the undo/redo stack.
  *
  * @param entity [IEntitySPtr &]
  * @param column [int]
  */
void MsaSubseqTableModel::onSubseqEntityDataChanged(const IEntitySPtr &entity, int column)
{
    ASSERT(msa_ != nullptr);
    ASSERT_X(boost::shared_dynamic_cast<AbstractSeq>(entity), "entity must be an AbstractSeq");
    int tableColumn = mapFromColumnAdapter(column);
    if (tableColumn == -1)
        return;

    const AbstractSeqSPtr &abstractSeq = boost::shared_static_cast<AbstractSeq>(entity);
    int row = msa_->indexOfAbstractSeq(abstractSeq);
    if (row == 0)
        return;

    QModelIndex changedIndex = index(row - 1, tableColumn);
    emit dataChanged(changedIndex, changedIndex);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param subseq [const Subseq *]
  * @param column [int]
  * @param role [int]
  * @returns QVariant
  */
QVariant MsaSubseqTableModel::data(const Subseq *subseq, int column, int role) const
{
    ASSERT(subseq != nullptr);

    switch (column)
    {
    case eStartColumn:
        return (role == Qt::DisplayRole) ? subseq->start() : QVariant();
    case eStopColumn:
        return (role == Qt::DisplayRole) ? subseq->stop() : QVariant();

    default:
        break;
    }

    if (columnAdapter_ != nullptr)
        return columnAdapter_->data(subseq->seqEntity_, column - eNumberOfSelfColumns, role);

    return QVariant();
}
