/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SubseqSliceModel.h"

#include "AdocTreeModel.h"
#include "RelatedTableModel.h"
#include "../AdocTreeNode.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * Initialize alphabet to eUnknownAlphabet and pointers to zero.
  *
  * @param parent [QObject *]
  */
SubseqSliceModel::SubseqSliceModel(QObject *parent) : SliceProxyModel(parent)
{
    alphabet_ = eUnknownAlphabet;
    subseqTable_ = 0;
    seqTable_ = 0;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Alphabet
  */
Alphabet SubseqSliceModel::alphabet() const
{
    return alphabet_;
}

/**
  * @param parent [const QModelIndex &] - unused
  * @returns int
  */
int SubseqSliceModel::columnCount(const QModelIndex & /* parent */) const
{
    if (!subseqTable_ || !seqTable_)
        return 0;

    return seqTable_->columnCount() + subseqTable_->columnCount();
}

/**
  * Convenience method that simply calls data(index, role) and provides a default implementation.
  *
  * @param index [const QModelIndex &]
  * @param role [int]
  * @param handled [bool &]
  * @returns QVariant
  * @see data(QModelIndex &, int, bool)
  */
QVariant SubseqSliceModel::data(const QModelIndex &index, int role) const
{
    bool handled;
    return data(index, role, handled);
}

/**
  * Much of the data handling for subseq and seq data can be processed here without the need for delegating
  * this to specialized classes (e.g. AminoSliceModel). This method primarily handles returning data for
  * display and data relevant to group / MSA nodes. If the data has been processed, handled is set to true;
  * otherwise it is set to false.
  *
  * @param index [const QModelIndex &]
  * @param role [int]
  * @param handled [bool &]
  * @returns QVariant
  */
QVariant SubseqSliceModel::data(const QModelIndex &index, int role, bool &handled) const
{
    handled = true;
    if (!isGoodIndex(index))
        return QVariant();

    // Technically, it should not be possible to reach this point, unless the model has been properly
    // initialized. This is because slice data is only added to slice_ if taggedSliceReady was called
    // correctly which always returns false if isReady() returns false. And indexes from this model
    // may only be created with the index function which checks that the index is in a valid location.
    AdocTreeNode *node = slice_.sourceNodes_.at(index.row());

    // All group and msa nodes get handled by the AdocTreeModel class
    if (isGroupOrMsa(node))
    {
        if (index.column() == primaryColumn())
            return sourceTreeModel()->indexFromNode(node).data(role);

        // Essential for sorting purposes that the QVariant returned is valid, but null
        return QVariant(QVariant::Bool);
    }

    // Safe to assume that the node is a SubseqAminoNode at this point
    if (role == Qt::DisplayRole)
    {
        if (index.column() < subseqTable_->columnCount())
            return subseqTable_->data(node->fkId_, index.column());

        // Else, this must be a seq level property, request its seq id and using that request the column data
        int seqId = subseqTable_->data(node->fkId_, subseqTable_->relatedField(RelatedTableModel::eRelationBelongsTo, seqTable_)).toInt();
        return seqTable_->data(seqId, index.column() - subseqTable_->columnCount());
    }
    else if (role == Qt::DecorationRole && index.column() == primaryColumn())
        return sourceTreeModel()->indexFromNode(node).data(role);

    handled = false;
    return QVariant();
}

/**
  * The header data simply returns the friendly field name for all valid columns.
  *
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant SubseqSliceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Also check that tables have been initialized
    if (!subseqTable_ || !seqTable_)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        if (section < subseqTable_->columnCount())
            return subseqTable_->friendlyFieldName(section);

        // Else, this must be a seq level column
        return seqTable_->friendlyFieldName(section - subseqTable_->columnCount());
    }
    else // orientation == Qt::Vertical; use this to indicate the row number. Section is 0-based, so we add 1 to make it 1-based.
        return section + 1;
}

/**
  * To properly initialize a SubseqSlice model, the following properties must have been set:
  * o Valid AdocTreeModel (SliceProxyModel::setTreeModel)
  * o Alphabet must not be eUnknownAlphabet
  * o Valid (non-zero) subseqTable_ and seqTable_
  *
  * Even though a model may have all of these in place, it does not mean that the composed classes
  * (e.g. subseqTable_ / seqTable_) have been properly configured. Thus, it is possible for this function
  * to return true, yet downstream methods (e.g. data()) not return the expected result.
  *
  * @returns bool
  */
bool SubseqSliceModel::isInitialized() const
{
    return alphabet_ != eUnknownAlphabet
            && subseqTable_
            && seqTable_
            && sourceTreeModel();
}

/**
  * This method always resets and clears the model.
  *
  * Because we are connecting two additional models to comprise the data in this model, it is necessary to
  * watch when they are modified and react accordingly. Specifically:
  * o modelReset -> clearAndReset
  * o dataChanged -> dataChanged
  *
  * Unecessary to handle the sourceChanged signal because this will also issue a reset signal which will
  * be captured above.
  * o sourceChanged -> clearAndReset; because the subseqTable or seqTable have different source parameters,
  *                    must remove all slice data and reset model
  *
  * @param subseqTable [RelatedTableModel *]
  * @param seqTable [TableModel *]
  */
void SubseqSliceModel::setSourceTables(Alphabet alphabet, RelatedTableModel *subseqTable, TableModel *seqTable)
{
    alphabet_ = alphabet;

    if (subseqTable_)
    {
        disconnect(subseqTable_, SIGNAL(loadDone(int)), this, SLOT(subseqsLoadDone(int)));
        disconnect(subseqTable_, SIGNAL(loadError(QString,int)), this, SLOT(subseqsLoadError(QString,int)));

        disconnect(subseqTable_, SIGNAL(friendlyFieldNamesChanged(int,int)), this, SLOT(subseqFriendlyFieldNamesChanged(int,int)));
        disconnect(subseqTable_, SIGNAL(modelReset()), this, SLOT(refreshSourceParent()));
        disconnect(subseqTable_, SIGNAL(dataChanged(int,int)), this, SLOT(subseqDataChanged(int,int)));
    }

    if (seqTable_)
    {
        disconnect(seqTable_, SIGNAL(friendlyFieldNamesChanged(int,int)), this, SLOT(seqFriendlyFieldNamesChanged(int,int)));
        disconnect(seqTable_, SIGNAL(modelReset()), this, SLOT(refreshSourceParent()));
        disconnect(seqTable_, SIGNAL(dataChanged(int,int)), this, SLOT(seqDataChanged(int,int)));
    }

    subseqTable_ = subseqTable;
    seqTable_ = seqTable;

    if (subseqTable_)
    {
        connect(subseqTable_, SIGNAL(loadDone(int)), SLOT(subseqsLoadDone(int)));
        connect(subseqTable_, SIGNAL(loadError(QString,int)), SLOT(subseqsLoadError(QString,int)));

        connect(subseqTable_, SIGNAL(friendlyFieldNamesChanged(int,int)), this, SLOT(subseqFriendlyFieldNamesChanged(int,int)));
        connect(subseqTable_, SIGNAL(modelReset()), SLOT(refreshSourceParent()));
        connect(subseqTable_, SIGNAL(dataChanged(int,int)), SLOT(subseqDataChanged(int,int)));
    }

    if (seqTable_)
    {
        connect(seqTable_, SIGNAL(friendlyFieldNamesChanged(int,int)), this, SLOT(seqFriendlyFieldNamesChanged(int,int)));
        connect(seqTable_, SIGNAL(modelReset()), SLOT(refreshSourceParent()));
        connect(seqTable_, SIGNAL(dataChanged(int,int)), SLOT(seqDataChanged(int,int)));
    }

    clear();
}

/**
  * Generic setData function that maps all setData requests to source AdocTreeModel if the column is the
  * primary column or the relevant TableModel (subseqTable_ or seqTable_).
  *
  * Because this model functions as an adaptor to the underlying data, it does not emit any dataChanged
  * events. Rather, it pushes the request to the appropriate TableModel, which attempts to update the
  * actual stored data and if successful will emit a dataChanged event. This in turn will call either
  * seqDataChanged or subseqDataChanged (see the signal/slots established within setSourceTables), which
  * will in turn emit the proper dataChanged event for this model.
  *
  * Conditions which will do nothing and return false:
  * o role is not EditRole
  * o index is not a valid index
  * o Either subseqTable or the seqTable is null
  *
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  * @returns bool
  * @see setSourceTables()
  */
bool SubseqSliceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (!isGoodIndex(index))
        return false;

    AdocTreeNode *node = slice_.sourceNodes_.at(index.row());
    if (isGroupOrMsa(node))
    {
        if (index.column() == primaryColumn())
        {
            QModelIndex sourceIndex = sourceTreeModel()->indexFromNode(node);
            return sourceTreeModel()->setData(sourceIndex, value, role);
        }

        return false;
    }

    // Also check that tables have been initialized
    if (!subseqTable_ || !seqTable_)
        return false;

    if (index.column() < subseqTable_->columnCount())
        return subseqTable_->setData(node->fkId_, index.column(), value);

    // Else, this is a seq level property
    return seqTable_->setData(node->fkId_, index.column() - subseqTable_->columnCount(), value);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * This virtual signal is called from the SliceProxyModel base class when at least one node has been
  * prepared for addition to the model. It is stored in the taggedSlice_ protected member and can be
  * accessed via tag.
  *
  * Immediately transfer group and msa nodes from the taggedSlice to slice_ and if any subseq nodes remain,
  * submit a load request to the subseq table.
  *
  * @param tag [int]
  */
void SubseqSliceModel::taggedSliceCreated(int tag)
{
    ASSERT(taggedSlices_.contains(tag));
    if (!taggedSlices_.contains(tag))       // Release mode guard
        return;

    if (!isInitialized())
    {
        qWarning("%s: Tagged slice created when model is not ready", __FUNCTION__);
        taggedSlices_.remove(tag);
        return;
    }

    Slice groupsAndMsas;

    QList<int> subseqIds;
    Slice &slice = taggedSlices_[tag];
    for (int i=slice.count()-1; i>= 0; --i)
    {
        AdocTreeNode *node = slice.sourceNodes_.at(i);

        if (node->nodeType_ == AdocTreeNode::GroupType
            || (node->nodeType_ == AdocTreeNode::MsaAminoType && alphabet_ == eAminoAlphabet)
            || (node->nodeType_ == AdocTreeNode::MsaDnaType   && alphabet_ == eDnaAlphabet)
            || (node->nodeType_ == AdocTreeNode::MsaRnaType   && alphabet_ == eRnaAlphabet) )
        {
            groupsAndMsas.sourceNodes_      << slice.sourceNodes_.takeAt(i);
            groupsAndMsas.sourceRowIndices_ << slice.sourceRowIndices_.takeAt(i);
        }
        else
        {
            switch (alphabet_)
            {
            case eAminoAlphabet:
                if (node->nodeType_ == AdocTreeNode::SubseqAminoType)
                    subseqIds << node->fkId_;
                break;
            case eDnaAlphabet:
                if (node->nodeType_ == AdocTreeNode::SubseqDnaType)
                    subseqIds << node->fkId_;
                break;
            case eRnaAlphabet:
                if (node->nodeType_ == AdocTreeNode::SubseqRnaType)
                    subseqIds << node->fkId_;
                break;

            default:
                break;
            }
        }
    }

    if (groupsAndMsas.count())
        appendSlice(groupsAndMsas);

    if (subseqIds.isEmpty())
    {
        taggedSlices_.remove(tag);
        return;
    }

    subseqTable_->load(subseqIds, tag);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * Must emit a dataChanged signal whenever one of the source subseq records also present in this model
  * has been changed. Note that the column parameter is in the range 0..n where n is the number of columns
  * in seqTable_. Because this class places all subseq columns prior to seq columns, it is necessary to
  * map the seq level column to its corresponding column in this model.
  *
  * Currently using a naive search algorithm O(n) that simply loops through all records and emits the
  * dataChanged signal if the id matches.
  *
  * OPTIMIZATION: Utilize index for matching a TableModel id to its corresponding row
  *
  * @param id [int]
  * @param column [int]
  */
void SubseqSliceModel::seqDataChanged(int id, int column)
{
    ASSERT(seqTable_);
    ASSERT(subseqTable_);
    if (!seqTable_ || !subseqTable_) // Release mode guard
        return;

    ASSERT(column > 0 && column < seqTable_->columnCount());
    if (column < 0 || column >= seqTable_->columnCount())   // Release mode guard
        return;

    for (int i=0, z=slice_.count(); i<z; ++i)
    {
        if (slice_.sourceNodes_.at(i)->fkId_ == id)
        {
            // Map the column
            QModelIndex targetIndex = index(i, column + subseqTable_->columnCount());
            emit dataChanged(targetIndex, targetIndex);

            // Since there can only be one row identified by id and it has been found,
            // nothing else to do
            return;
        }
    }
}

/**
  * Must map the first and last columns of subseqTable to this model. Because all subseq columns
  * are displayed prior to any seq columns, must add the number of columns in the subseq table
  * to both first and last.
  *
  * @param first [int]
  * @param last [int]
  */
void SubseqSliceModel::seqFriendlyFieldNamesChanged(int first, int last)
{
    ASSERT(seqTable_);
    ASSERT(first >= 0 && first < seqTable_->columnCount());
    ASSERT(last >= 0 && last < seqTable_->columnCount());

    emit headerDataChanged(Qt::Horizontal, subseqTable_->columnCount() + first, subseqTable_->columnCount() + last);
}

/**
  * Must emit a dataChanged signal whenever one of the source subseq records also present in this model
  * has been changed. Note that the column parameter is in the range 0..n where n is the number of columns
  * in subseqTable_. Because this class places all subseq columns prior to seq columns, it directly maps
  * to the columns in this model.
  *
  * Currently using a naive search algorithm O(n) that simply loops through all records and emits the
  * dataChanged signal if the id matches.
  *
  * OPTIMIZATION: Utilize index for matching a TableModel id to its corresponding row
  *
  * @param id [int]
  * @param column [int]
  */
void SubseqSliceModel::subseqDataChanged(int id, int column)
{
    ASSERT(subseqTable_);
    if (!subseqTable_) // Release mode guard
        return;

    ASSERT(column > 0 && column < subseqTable_->columnCount());
    if (column < 0 || column >= subseqTable_->columnCount())   // Release mode guard
        return;

    for (int i=0, z=slice_.count(); i<z; ++i)
    {
        if (slice_.sourceNodes_.at(i)->fkId_ == id)
        {
            QModelIndex targetIndex = index(i, column);
            emit dataChanged(targetIndex, targetIndex);

            // Since there can only be one row identified by id and it has been found,
            // nothing else to do
            return;
        }
    }
}

/**
  * Must map the first and last columns of subseqTable to this model. Because all subseq columns
  * are displayed prior to any seq columns, this should be a direct mapping with no alteration.
  *
  * @param first [int]
  * @param last [int]
  */
void SubseqSliceModel::subseqFriendlyFieldNamesChanged(int first, int last)
{
    ASSERT(subseqTable_);
    ASSERT(first >= 0 && first < subseqTable_->columnCount());
    ASSERT(last >= 0 && last < subseqTable_->columnCount());

    emit headerDataChanged(Qt::Horizontal, first, last);
}

/**
  * Append the subseqs associated with tag in the taggedSlice_ member to the slice_.
  *
  * @param tag [int]
  */
void SubseqSliceModel::subseqsLoadDone(int tag)
{
//    ASSERT(taggedSlices_.contains(tag));
    if (!taggedSlices_.contains(tag))       // Release mode guard
        return;

    appendSlice(taggedSlices_.take(tag));
}

/**
  * Remove the taggedSlice associated with tag.
  *
  * TODO: Emit an error signal
  *
  * @param error [const QString &]
  * @param tag [int]
  */
void SubseqSliceModel::subseqsLoadError(const QString &error, int tag)
{
//    ASSERT(taggedSlices_.contains(tag));
    if (!taggedSlices_.contains(tag))       // Release mode guard
        return;

    // Remove from list and report error somehow...
    taggedSlices_.take(tag);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * See class introduction for filtering rules.
  *
  * @param node [AdocTreeNode *]
  * @returns bool
  */
bool SubseqSliceModel::filterAcceptsNode(AdocTreeNode *node) const
{
    if (node->nodeType_ == AdocTreeNode::GroupType)
        return true;

    switch (alphabet_)
    {
    case eAminoAlphabet:
        return node->nodeType_ == AdocTreeNode::SubseqAminoType || node->nodeType_ == AdocTreeNode::MsaAminoType;
    case eDnaAlphabet:
        return node->nodeType_ == AdocTreeNode::SubseqDnaType || node->nodeType_ == AdocTreeNode::MsaDnaType;
    case eRnaAlphabet:
        return node->nodeType_ == AdocTreeNode::SubseqRnaType || node->nodeType_ == AdocTreeNode::MsaRnaType;

    default:
        return false;
    }
}

/**
  * @param node [AdocTreeNode *]
  * @returns bool
  */
bool SubseqSliceModel::isGroupOrMsa(AdocTreeNode *node) const
{
    ASSERT(node);

    return node->nodeType_ == AdocTreeNode::GroupType
           || node->nodeType_ == AdocTreeNode::MsaAminoType
           || node->nodeType_ == AdocTreeNode::MsaDnaType
           || node->nodeType_ == AdocTreeNode::MsaRnaType;
}
