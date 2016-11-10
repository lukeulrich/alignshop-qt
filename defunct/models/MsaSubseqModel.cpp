/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "MsaSubseqModel.h"
#include "RelatedTableModel.h"
#include "TagGenerator.h"

#include <QtDebug>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static member initialization
const QStringList MsaSubseqModel::coreFields_(QStringList() << "subseq_id" << "start" << "stop");
const QStringList MsaSubseqModel::friendlyCoreFieldNames_(QStringList() << "Subseq ID" << "Start" << "Stop");
const int MsaSubseqModel::kCoreSubseqIdColumn_ = 0;
const int MsaSubseqModel::kCoreSubseqStartColumn_ = 1;
const int MsaSubseqModel::kCoreSubseqStopColumn_ = 2;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
MsaSubseqModel::MsaSubseqModel(QObject *parent) : QAbstractTableModel(parent)
{
    // Set pointers to zero
    msa_ = 0;
    subseqTable_ = 0;
    seqTable_ = 0;

    // Initialize tags to 0
    subseqLoadTag_ = 0;
    seqLoadTag_ = 0;
}

/**
  */
MsaSubseqModel::~MsaSubseqModel()
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Always returns at least 3 columns, and more if subseqTable_ and seqTable_ are defined.
  *
  * @param parent [const QModelIndex &]
  * @returns int
  */
int MsaSubseqModel::columnCount(const QModelIndex & /* parent */) const
{
    int nColumns = 3;
    if (subseqTable_)
    {
        nColumns += subseqTable_->columnCount();
        if (seqTable_)
            nColumns += seqTable_->columnCount();
    }

    return nColumns;
}

/**
  *
  *
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant MsaSubseqModel::data(const QModelIndex &index, int role) const
{
    if (!isGoodIndex(index))
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() < 3)
    {
        switch (index.column())
        {
        case kCoreSubseqIdColumn_:
            return msa_->at(index.row()+1)->id();
        case kCoreSubseqStartColumn_:
            return msa_->at(index.row()+1)->start();
        case kCoreSubseqStopColumn_:
            return msa_->at(index.row()+1)->stop();
        default:
            return QVariant();
        }
    }

    if (subseqTable_)
    {
        if (index.column() - 3 < subseqTable_->columnCount())
            return subseqTable_->data(msa_->at(index.row()+1)->id(), index.column() - 3);

        if (seqTable_)
        {
            if (index.column() - 3 < columnCount())
            {
                int seqId = subseqTable_->data(msa_->at(index.row()+1)->id(), subseqTable_->relatedField(RelatedTableModel::eRelationBelongsTo, seqTable_)).toInt();
                return seqTable_->data(seqId, index.column() - 3 - subseqTable_->columnCount());
            }
        }
    }

    return QVariant();
}

/**
  *
  *
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags MsaSubseqModel::flags(const QModelIndex & /* index */) const
{
    return 0;
}

/**
  * Depending on the actual value for section and whether any annotation tables have been
  * defined, it is mapped in the following manner:
  * o [0 .. 2] -> core column
  *
  * o If subseqTable_:
  *    [3 .. subseqTable_->columnCount() - 1] -> subseqTable->friendlyFieldName
  *
  *    o If seqTable_:
  *       [subseqTable_->columntCount() .. columnCount() - 1] -> seqTable->friendlyFieldName
  *
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant MsaSubseqModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        if (section < 0)
            return QVariant();

        if (section < 3)
            return friendlyCoreFieldNames_.at(section);

        if (!subseqTable_)
            return QVariant();

        if (section < 3 + subseqTable()->columnCount())
            return subseqTable()->friendlyFieldName(section - 3);

        if (!seqTable_)
            return QVariant();

        if (section < columnCount())
            return seqTable()->friendlyFieldName(section - 3 - subseqTable_->columnCount());

        return QVariant();
    }
    else    // orientation == Qt::Vertical
        return section + 1;
}

/**
  *
  *
  * @param parent [const QModelIndex &]
  * @returns int
  */
int MsaSubseqModel::rowCount(const QModelIndex & /* parent */) const
{
    if (!msa_)
        return 0;

    return msa_->subseqCount();
}

/**
  *
  *
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  * @returns bool
  */
//bool MsaSubseqModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    return false;
//}

/**
  *
  *
  * @param column [int]
  * @param order [Qt::SortOrder]
  */
void MsaSubseqModel::sort(int column, Qt::SortOrder order)
{
}

/**
  *
  *
  * @returns Qt::DropActions
  */
Qt::DropActions MsaSubseqModel::supportedDragActions() const
{
    return 0;
}

/**
  *
  *
  * @returns Qt::DropActions
  */
Qt::DropActions MsaSubseqModel::supportedDropActions() const
{
    return 0;
}

/**
  *
  *
  * @returns QStringList
  */
QStringList MsaSubseqModel::mimeTypes() const
{
    return QStringList();
}

/**
  *
  *
  * @param indexes [const QModelIndexList &]
  * @returns QMimeData *
  */
QMimeData *MsaSubseqModel::mimeData(const QModelIndexList &indexes) const
{
    return 0;
}

/**
  *
  *
  * @param data [const QMimeData *]
  * @param action [Qt::DropAction]
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool MsaSubseqModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return false;
}

/**
  * TODO: Refactor into common base class. Currently same implementation is found in SliceProxyModel
  * UNTESTED HERE!
  *
  * @param index [const QModelIndex &]
  * @returns bool
  */
bool MsaSubseqModel::isGoodIndex(const QModelIndex &index) const
{
    if (!index.isValid()
        || index.model() != this
        || index.row() >= rowCount()
        || index.column() >= columnCount())
    {
        return false;
    }

    return true;
}

/**
  * @returns bool
  */
bool MsaSubseqModel::isReady() const
{
    return msa_ != 0;
}

/**
  *
  *
  * @returns const Msa *
  */
const Msa *MsaSubseqModel::msa() const
{
    return msa_;
}

/**
  * Sets the source msa (msa_) for this model to msa and resets the model. Also, kick off a request
  * for all subseq data.
  *
  * @param msa [Msa *]
  */
void MsaSubseqModel::setMsa(Msa *msa)
{
    beginResetModel();
    if (msa_)
    {
        // Signal -> signal
        disconnect(msa_, SIGNAL(msaReset()),                               this, SIGNAL(modelReset()));
        disconnect(msa_, SIGNAL(subseqsAboutToBeSorted()),                 this, SIGNAL(layoutAboutToBeChanged()));
        disconnect(msa_, SIGNAL(subseqsSorted()),                          this, SIGNAL(layoutChanged()));

        // Signal -> slot
        disconnect(msa_, SIGNAL(regionSlid(int,int,int,int,int,int,int)),  this, SLOT(onMsaRegionSlid(int,int,int,int)));
        disconnect(msa_, SIGNAL(subseqAboutToBeSwapped(int,int)),          this, SLOT(onMsaSubseqAboutToBeSwapped(int,int)));
        disconnect(msa_, SIGNAL(subseqsAboutToBeInserted(int,int)),        this, SLOT(onMsaSubseqsAboutToBeInserted(int,int)));
        disconnect(msa_, SIGNAL(subseqsAboutToBeMoved(int,int,int)),       this, SLOT(onMsaSubseqsAboutToBeMoved(int,int,int)));
        disconnect(msa_, SIGNAL(subseqsAboutToBeRemoved(int,int)),         this, SLOT(onMsaSubseqsAboutToBeRemoved(int,int)));
        disconnect(msa_, SIGNAL(subseqsInserted(int,int)),                 this, SLOT(onMsaSubseqsInserted(int,int)));
        disconnect(msa_, SIGNAL(subseqsMoved(int,int,int)),                this, SLOT(onMsaSubseqsMoved(int,int,int)));
        disconnect(msa_, SIGNAL(subseqsRemoved(int,int)),                  this, SLOT(onMsaSubseqsRemoved(int,int)));
        disconnect(msa_, SIGNAL(subseqSwapped(int,int)),                   this, SLOT(onMsaSubseqSwapped(int,int)));
    }

    msa_ = msa;

    if (msa_)
    {
        // Signal -> signal
        connect(msa_, SIGNAL(msaReset()),                               SIGNAL(modelReset()));
        connect(msa_, SIGNAL(subseqsAboutToBeSorted()),                 SIGNAL(layoutAboutToBeChanged()));
        connect(msa_, SIGNAL(subseqsSorted()),                          SIGNAL(layoutChanged()));

        // Signal -> slot
        connect(msa_, SIGNAL(regionSlid(int,int,int,int,int,int,int)),  SLOT(onMsaRegionSlid(int,int,int,int)));
        connect(msa_, SIGNAL(subseqAboutToBeSwapped(int,int)),          SLOT(onMsaSubseqAboutToBeSwapped(int,int)));
        connect(msa_, SIGNAL(subseqsAboutToBeInserted(int,int)),        SLOT(onMsaSubseqsAboutToBeInserted(int,int)));
        connect(msa_, SIGNAL(subseqsAboutToBeMoved(int,int,int)),       SLOT(onMsaSubseqsAboutToBeMoved(int,int,int)));
        connect(msa_, SIGNAL(subseqsAboutToBeRemoved(int,int)),         SLOT(onMsaSubseqsAboutToBeRemoved(int,int)));
        connect(msa_, SIGNAL(subseqsInserted(int,int)),                 SLOT(onMsaSubseqsInserted(int,int)));
        connect(msa_, SIGNAL(subseqsMoved(int,int,int)),                SLOT(onMsaSubseqsMoved(int,int,int)));
        connect(msa_, SIGNAL(subseqsRemoved(int,int)),                  SLOT(onMsaSubseqsRemoved(int,int)));
        connect(msa_, SIGNAL(subseqSwapped(int,int)),                   SLOT(onMsaSubseqSwapped(int,int)));
    }
    endResetModel();

    if (msa_ && subseqTable_)
    {
        subseqLoadTag_ = TagGenerator::nextValue();
        subseqTable_->load(msa_->subseqIds(), subseqLoadTag_);
    }
}

/**
  * The exact response and emitted signals depends upon the supplied values and any previously defined
  * annotation tables. The only required argument is subseqTable (seqTable is null by default) because
  * there will be one subseq record for each row. On theh other hand, a non-null seqTable argument is
  * only updated if subseqTable is not null.
  *
  * Attaches/detaches as needed to the dataChanged and modelReset signals of subseqTable and seqTable.
  * Additionally, because the columns to be displayed strictly depend on the arguments passed to this
  * method, the columnsAboutToBeRemoved/columnsRemoved is called immediately if one or more annotation
  * columns should not be shown that was previously displayed.
  *
  * Because the annotation may not have been loaded for this Msa's subseqs, a load request is submitted
  * to subseqTable_ (if set). If successful, it will then emit the columnsAboutToBeInserted/columnsInserted
  * signals.
  *
  * @param subseqTable [RelatedTableModel *]
  * @param seqTable [TableModel *]
  */
void MsaSubseqModel::setAnnotationTables(RelatedTableModel *subseqTable, TableModel *seqTable)
{
    int nColumnsToRemove = 0;

    if (seqTable_)
    {
        disconnect(seqTable_, SIGNAL(modelReset()), this, SLOT(seqTableReset()));
        disconnect(seqTable_, SIGNAL(dataChanged(int,int)), this, SLOT(seqDataChanged(int,int)));
        disconnect(seqTable_, SIGNAL(friendlyFieldNamesChanged(int,int)), this, SLOT(seqFriendlyFieldNamesChanged(int,int)));
        disconnect(seqTable_, SIGNAL(loadDone(int)), this, SLOT(seqsOnlyLoadDone(int)));
        disconnect(seqTable_, SIGNAL(loadError(QString,int)), this, SLOT(seqsOnlyLoadError(QString,int)));

        nColumnsToRemove += seqTable_->columnCount();
    }

    if (subseqTable_)
    {
        disconnect(subseqTable_, SIGNAL(modelReset()), this, SLOT(subseqTableReset()));
        disconnect(subseqTable_, SIGNAL(dataChanged(int,int)), this, SLOT(subseqDataChanged(int,int)));
        disconnect(subseqTable_, SIGNAL(friendlyFieldNamesChanged(int,int)), this, SLOT(subseqFriendlyFieldNamesChanged(int,int)));
        disconnect(subseqTable_, SIGNAL(loadDone(int)), this, SLOT(subseqsLoadDone(int)));
        disconnect(subseqTable_, SIGNAL(loadError(QString,int)), this, SLOT(subseqsLoadError(QString,int)));

        nColumnsToRemove += subseqTable_->columnCount();
    }

    if (nColumnsToRemove)
    {
        beginRemoveColumns(QModelIndex(), 3, 3 + nColumnsToRemove - 1);
        endRemoveColumns();
    }

    subseqTable_ = subseqTable;
    seqTable_ = 0;

    int nColumnsToInsert = 0;
    if (subseqTable_)
    {
        connect(subseqTable_, SIGNAL(modelReset()), SLOT(subseqTableReset()));
        connect(subseqTable_, SIGNAL(dataChanged(int,int)), SLOT(subseqDataChanged(int,int)));
        connect(subseqTable_, SIGNAL(friendlyFieldNamesChanged(int,int)), this, SLOT(subseqFriendlyFieldNamesChanged(int,int)));
        connect(subseqTable_, SIGNAL(loadDone(int)), this, SLOT(subseqsLoadDone(int)));
        connect(subseqTable_, SIGNAL(loadError(QString,int)), this, SLOT(subseqsLoadError(QString,int)));

        nColumnsToInsert += subseqTable_->columnCount();

        if (seqTable)
        {
            seqTable_ = seqTable;
            connect(seqTable_, SIGNAL(modelReset()), SLOT(seqTableReset()));
            connect(seqTable_, SIGNAL(dataChanged(int,int)), SLOT(seqDataChanged(int,int)));
            connect(seqTable_, SIGNAL(friendlyFieldNamesChanged(int,int)), this, SLOT(seqFriendlyFieldNamesChanged(int,int)));
            connect(seqTable_, SIGNAL(loadDone(int)), this, SLOT(seqsOnlyLoadDone(int)));
            connect(seqTable_, SIGNAL(loadError(QString,int)), this, SLOT(seqsOnlyLoadError(QString,int)));

            nColumnsToInsert += seqTable_->columnCount();
        }

        // If a Msa has been configured, submit a load request for all annotation data relating to these subseqs
        // When complete will push a dataChanged signal for these.
        if (msa_)
        {
            subseqLoadTag_ = TagGenerator::nextValue();
            subseqTable_->load(msa_->subseqIds(), subseqLoadTag_);
        }
    }

    if (nColumnsToInsert)
    {
        beginInsertColumns(QModelIndex(), 3, 3 + nColumnsToInsert - 1);
        endInsertColumns();
    }
}

/**
  *
  *
  * @returns const TableModel *
  */
const TableModel *MsaSubseqModel::seqTable() const
{
    return seqTable_;
}

/**
  *
  *
  * @returns const RelatedTableModel *
  */
const RelatedTableModel *MsaSubseqModel::subseqTable() const
{
    return subseqTable_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * Unused!
  *
  * @param left [int]
  * @param top [int]
  * @param right [int]
  * @param bottom [int]
  */
void MsaSubseqModel::onMsaRegionSlid(int /* left */, int /* top */, int /* right */, int /* bottom */)
{
}

/**
  *
  *
  * @param start [int]
  * @param end [int]
  */
void MsaSubseqModel::onMsaSubseqsAboutToBeInserted(int start, int end)
{
}

/**
  * Important! The move indices supplied by the Msa class are 1-based. Moreover, QList::move utilizes a
  * different index naming method than QAbstractItemModel::moveRows when moving earlier rows further
  * down in the array. Consequently, it is necessary to properly map the indices of a Msa subseq move
  * event to their corresponding indices of QAbstractItemModel. For details, see log.txt (7 December 2010).
  *
  * @param start [int]
  * @param end [int]
  * @param target [int]
  */
void MsaSubseqModel::onMsaSubseqsAboutToBeMoved(int start, int end, int target)
{
    if (target < start)
        beginMoveRows(QModelIndex(), start-1, end-1, QModelIndex(), target-1);
    else
        beginMoveRows(QModelIndex(), start-1, end-1, QModelIndex(), target+1);
}

/**
  *
  *
  * @param start [int]
  * @param end [int]
  */
void MsaSubseqModel::onMsaSubseqsAboutToBeRemoved(int start, int end)
{
}

/**
  * @param first [int]
  * @param second [int]
  */
void MsaSubseqModel::onMsaSubseqAboutToBeSwapped(int first, int second)
{
    if (first < second)
        beginMoveRows(QModelIndex(), first-1, first-1, QModelIndex(), second);
    else
        beginMoveRows(QModelIndex(), first-1, first-1, QModelIndex(), second-1);
}

/**
  *
  *
  * @param start [int]
  * @param end [int]
  */
void MsaSubseqModel::onMsaSubseqsInserted(int start, int end)
{
}

/**
  *
  *
  * @param start [int]
  * @param end [int]
  * @param target [int]
  */
void MsaSubseqModel::onMsaSubseqsMoved(int /* start */, int /* end */, int /* target */)
{
    endMoveRows();
}

/**
  *
  *
  * @param start [int]
  * @param end [int]
  */
void MsaSubseqModel::onMsaSubseqsRemoved(int start, int end)
{
}

/**
  * @param first [int]
  * @param second [int]
  */
void MsaSubseqModel::onMsaSubseqSwapped(int /* first */, int /* second */)
{
    endMoveRows();
}

/**
  * @param id [int]
  * @param column [int]
  * @see subseqDataChanged()
  */
void MsaSubseqModel::seqDataChanged(int id, int column)
{
    ASSERT(subseqTable_);
    ASSERT(seqTable_);
    ASSERT(column >= 0);
    ASSERT(column < seqTable_->columnCount());

    for (int i=0, z=rowCount(); i<z; ++i)
    {
        // Msa is 1-based - thus must add one to its subseq accessor method
        int seqId = subseqTable_->data(msa_->at(i+1)->id(), subseqTable_->relatedField(RelatedTableModel::eRelationBelongsTo, seqTable_)).toInt();
        if (seqId == id)
        {
            QModelIndex changedIndex = index(i, column + 3 + subseqTable_->columnCount());
            emit dataChanged(changedIndex, changedIndex);

            // Because a given seq may map to multiple subseqs, it is necessary to iterate
            // through all subseq records rather than immediately stopping after updating
            // the first one (as is done for subseqs)
        }
    }
}

/**
  * Must map the first and last columns of subseqTable to this model. Because all core and subseq
  * columns are displayed prior to any seq columns, must add 3 + the number of columns in the
  * subseq table to both first and last.
  *
  * @param first [int]
  * @param last [int]
  */
void MsaSubseqModel::seqFriendlyFieldNamesChanged(int first, int last)
{
    ASSERT(seqTable_);
    ASSERT(first >= 0 && first < seqTable_->columnCount());
    ASSERT(last >= 0 && last < seqTable_->columnCount());

    emit headerDataChanged(Qt::Horizontal, 3 + subseqTable_->columnCount() + first, 3 + subseqTable_->columnCount() + last);
}

/**
  * Must build a list of unique sequence identifers for each subseq in the MSA and submit load request
  * for these.
  */
void MsaSubseqModel::seqTableReset()
{
    ASSERT(seqTable_);
    ASSERT(subseqTable_);

    if (rowCount() == 0)
        return;

    emit dataChanged(index(0, 3 + subseqTable_->columnCount()), index(rowCount()-1, columnCount()-1));

    // Build unique list of seq ids
    QSet<int> seqIds;
    for (int i=0, z = rowCount(); i<z; ++i)
    {
        int seqId = subseqTable_->data(msa_->at(i+1)->id(), subseqTable_->relatedField(RelatedTableModel::eRelationBelongsTo, seqTable_)).toInt();
        ASSERT(seqId > 0);

        seqIds << seqId;
    }

    seqLoadTag_ = TagGenerator::nextValue();
    seqTable_->load(seqIds.toList(), seqLoadTag_);
}

/**
  * @param tag [int]
  */
void MsaSubseqModel::seqsOnlyLoadDone(int tag)
{
    // In the rare event, that a load request completes and somehow the seqTable is cleared
    // before this method is called, reset the load tag and return without doing anything.
    //
    // Additionally, if there are no sequences for whatever reason, reset the load tag
    if ((!seqTable_ && seqLoadTag_ != 0)
        || rowCount() == 0)
    {
        seqLoadTag_ = 0;
        return;
    }

    if (tag != seqLoadTag_)
        return;

    if (columnCount() > 3 + subseqTable_->columnCount())
        emit dataChanged(index(0, 3 + subseqTable_->columnCount()), index(rowCount()-1, columnCount()-1));

    subseqLoadTag_ = 0;
}

/**
  * @param error [const QString &]
  * @param tag [int]
  */
void MsaSubseqModel::seqsOnlyLoadError(const QString &error, int /* tag */)
{
    qDebug() << "MsaSubseqModel::seqsOnlyLoadError:" << error;

    seqLoadTag_ = 0;
}

/**
  * Note, column pertains to the subseqTable_ column number. Not this model's column numbering
  * scheme.
  *
  * @param id [int]
  * @param column [int]
  */
void MsaSubseqModel::subseqDataChanged(int id, int column)
{
    ASSERT(subseqTable_);
    ASSERT(column >= 0);
    ASSERT(column < subseqTable_->columnCount());

    for (int i=0, z=rowCount(); i<z; ++i)
    {
        // Msa is 1-based - thus must add one to its subseq accessor method
        if (msa_->at(i+1)->id() == id)
        {
            QModelIndex changedIndex = index(i, column + 3);
            emit dataChanged(changedIndex, changedIndex);
            return;
        }
    }
}

/**
  * Must map the first and last columns of subseqTable to this model. Because all subseq column
  * data is displayed immediately after the first three core columns, mapping first and last
  * simply entails adding 3, respectively.
  *
  * @param first [int]
  * @param last [int]
  */
void MsaSubseqModel::subseqFriendlyFieldNamesChanged(int first, int last)
{
    ASSERT(subseqTable_);
    ASSERT(first >= 0 && first < subseqTable_->columnCount());
    ASSERT(last >= 0 && last < subseqTable_->columnCount());

    emit headerDataChanged(Qt::Horizontal, 3 + first, 3 + last);
}

/**
  * Tell view that all annotated data has changed and submit a data reload request for all the subseq
  * ids belonging to this msa. Should only emit dataChanged if there is a defined msa and at least one
  * row.
  *
  */
void MsaSubseqModel::subseqTableReset()
{
    ASSERT(subseqTable_);

    if (rowCount() == 0)
        return;

    emit dataChanged(index(0, 3), index(rowCount()-1, columnCount()-1));

    subseqLoadTag_ = TagGenerator::nextValue();
    subseqTable_->load(msa_->subseqIds(), subseqLoadTag_);
}

/**
  * Despite being called everytime a subseq is loaded, it is only processed if one or more subseqs
  * have been requested for loading. This method functions identically regardless if the load request
  * originated from the subseq model being reset or the setAnnotation method. Emits a dataChanged
  * event for all rows for all annotation columns.
  *
  * @param tag [int]
  */
void MsaSubseqModel::subseqsLoadDone(int tag)
{
    // In the rare event, that a load request completes and somehow the seqTable is cleared
    // before this method is called, reset the load tag and return without doing anything.
    //
    // Additionally, if there are no sequences for whatever reason, reset the load tag
    if ((!subseqTable_ && subseqLoadTag_ != 0)
        || rowCount() == 0)
    {
        subseqLoadTag_ = 0;
        return;
    }

    if (tag != subseqLoadTag_)
        return;

    if (columnCount() > 3)
        emit dataChanged(index(0, 3), index(rowCount() - 1, columnCount() - 1));

    subseqLoadTag_ = 0;
}

/**
  * @param error [const QString &]
  * @param tag [int]
  *
  * TODO: somehow report the error!
  */
void MsaSubseqModel::subseqsLoadError(const QString &error, int /* tag */)
{
    qDebug() << "MsaSubseqModel::subseqsLoadError:" << error;

    subseqLoadTag_ = 0;
}
