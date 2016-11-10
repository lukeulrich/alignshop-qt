/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "PrimerPairModel.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
PrimerPairModel::PrimerPairModel(QObject *parent)
    : AbstractPrimerPairModel(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void PrimerPairModel::clear()
{
    setPrimerPairs(QVector<PrimerPair>());
}

int PrimerPairModel::rowCount(const QModelIndex & /* parent */) const
{
    return primerPairs_.size();
}

QVector<PrimerPair> PrimerPairModel::primerPairs() const
{
    return primerPairs_;
}

void PrimerPairModel::setPrimerPairs(const QVector<PrimerPair> &newPrimerPairs)
{
    beginResetModel();
    primerPairs_ = newPrimerPairs;
    endResetModel();
}


// ------------------------------------------------------------------------------------------------
// Protected methods
PrimerPair *PrimerPairModel::primerPairPointerFromRow(const int row)
{
    if (!isValidRow(row))
        return nullptr;

    return &primerPairs_[row];
}

const PrimerPair *PrimerPairModel::primerPairPointerFromRow(const int row) const
{
    if (!isValidRow(row))
        return nullptr;

    return &primerPairs_.at(row);
}

