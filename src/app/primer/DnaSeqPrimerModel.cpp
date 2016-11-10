/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DnaSeqPrimerModel.h"
#include "DnaSeqPrimerVectorMutator.h"
#include "IPrimerMutator.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
DnaSeqPrimerModel::DnaSeqPrimerModel(DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator, IPrimerMutator *primerMutator, QObject *parent)
    : AbstractPrimerModel(primerMutator, parent),
      dnaSeqPrimerVectorMutator_(dnaSeqPrimerVectorMutator)
{
    observePrimerVectorMutator();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void DnaSeqPrimerModel::clear()
{
    beginResetModel();
    dnaSeq_->primers_.clear();
    endResetModel();
}

DnaSeqSPtr DnaSeqPrimerModel::dnaSeq() const
{
    return dnaSeq_;
}

Qt::ItemFlags DnaSeqPrimerModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = AbstractPrimerModel::flags(index);
    if (index.isValid() && index.column() == eNameColumn)
        flags |= Qt::ItemIsEditable;

    return flags;
}

int DnaSeqPrimerModel::rowCount(const QModelIndex & /* parent */) const
{
    if (dnaSeq_)
        return dnaSeq_->primers_.size();

    return 0;
}

PrimerVector DnaSeqPrimerModel::primers() const
{
    if (dnaSeq_)
        return dnaSeq_->primers_;

    return PrimerVector();
}

void DnaSeqPrimerModel::setDnaSeq(const DnaSeqSPtr &dnaSeq)
{
    if (dnaSeq_ == dnaSeq)
        return;

    beginResetModel();
    dnaSeq_ = dnaSeq;
    if (dnaSeq_)
        setBioString(dnaSeq_->abstractAnonSeq()->seq_.toBioString());
    else
        setBioString(BioString(eDnaGrammar));
    endResetModel();

    emit dnaSeqChanged(dnaSeq_);
}


// ------------------------------------------------------------------------------------------------
// Protected methods
Primer *DnaSeqPrimerModel::primerPointerFromRow(const int row)
{
    if (!isValidRow(row))
        return nullptr;

    return &dnaSeq_->primers_[row];
}

const Primer *DnaSeqPrimerModel::primerPointerFromRow(const int row) const
{
    if (!isValidRow(row))
        return nullptr;

    return &dnaSeq_->primers_.at(row);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
void DnaSeqPrimerModel::onDnaSeqPrimersAboutToBeAppended(const DnaSeqSPtr &dnaSeq, int startRow, int endRow)
{
    if (dnaSeq != dnaSeq_)
        return;

    beginInsertRows(QModelIndex(), startRow, endRow);
}

void DnaSeqPrimerModel::onDnaSeqPrimersAppended(const DnaSeqSPtr &dnaSeq, int /* startRow */, int /* endRow */)
{
    if (dnaSeq != dnaSeq_)
        return;

    endInsertRows();
}

void DnaSeqPrimerModel::onDnaSeqPrimersAboutToBeInserted(const DnaSeqSPtr &dnaSeq, int startRow, int endRow)
{
    if (dnaSeq != dnaSeq_)
        return;

    beginInsertRows(QModelIndex(), startRow, endRow);
}

void DnaSeqPrimerModel::onDnaSeqPrimersInserted(const DnaSeqSPtr &dnaSeq, int /* startRow */, int /* endRow */)
{
    if (dnaSeq != dnaSeq_)
        return;

    endInsertRows();
}

void DnaSeqPrimerModel::onDnaSeqPrimersAboutToBeRemoved(const DnaSeqSPtr &dnaSeq, int startRow, int endRow)
{
    if (dnaSeq != dnaSeq_)
        return;

    beginRemoveRows(QModelIndex(), startRow, endRow);
}

void DnaSeqPrimerModel::onDnaSeqPrimersRemoved(const DnaSeqSPtr &dnaSeq, int /* startRow */, int /* endRow */)
{
    if (dnaSeq != dnaSeq_)
        return;

    endRemoveRows();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
void DnaSeqPrimerModel::observePrimerVectorMutator()
{
    if (dnaSeqPrimerVectorMutator_ == nullptr)
        return;

    connect(dnaSeqPrimerVectorMutator_, SIGNAL(primersAboutToBeAppended(DnaSeqSPtr,int,int)), SLOT(onDnaSeqPrimersAboutToBeAppended(DnaSeqSPtr,int,int)));
    connect(dnaSeqPrimerVectorMutator_, SIGNAL(primersAppended(DnaSeqSPtr,int,int)), SLOT(onDnaSeqPrimersAppended(DnaSeqSPtr,int,int)));
    connect(dnaSeqPrimerVectorMutator_, SIGNAL(primersAboutToBeInserted(DnaSeqSPtr,int,int)), SLOT(onDnaSeqPrimersAboutToBeInserted(DnaSeqSPtr,int,int)));
    connect(dnaSeqPrimerVectorMutator_, SIGNAL(primersInserted(DnaSeqSPtr,int,int)), SLOT(onDnaSeqPrimersInserted(DnaSeqSPtr,int,int)));
    connect(dnaSeqPrimerVectorMutator_, SIGNAL(primersAboutToBeRemoved(DnaSeqSPtr,int,int)), SLOT(onDnaSeqPrimersAboutToBeRemoved(DnaSeqSPtr,int,int)));
    connect(dnaSeqPrimerVectorMutator_, SIGNAL(primersRemoved(DnaSeqSPtr,int,int)), SLOT(onDnaSeqPrimersRemoved(DnaSeqSPtr,int,int)));
}
