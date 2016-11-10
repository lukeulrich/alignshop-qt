/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFont>

#include "AbstractPrimerPairModel.h"
#include "../core/misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
AbstractPrimerPairModel::AbstractPrimerPairModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
BioString AbstractPrimerPairModel::bioString() const
{
    return bioString_;
}

int AbstractPrimerPairModel::columnCount(const QModelIndex & /* parent */) const
{
    return eNumberOfColumns;
}

QVariant AbstractPrimerPairModel::data(const QModelIndex &index, int role) const
{
    const PrimerPair *primerPair = primerPairPointerFromIndex(index);
    if (primerPair == nullptr)
        return QVariant();

    int primerPairColumn = index.column();

    switch (role)
    {
    case Qt::DisplayRole:       return displayRoleData(*primerPair, primerPairColumn);
    case Qt::FontRole:          return fontRoleData(primerPairColumn);
    case Qt::TextAlignmentRole: return textAlignmentRoleData(primerPairColumn);

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerPairModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return verticalHeaderData(section);

    return horizontalHeaderData(section);
}

bool AbstractPrimerPairModel::isValidRow(const int row) const
{
    return row >= 0 && row < rowCount();
}

PrimerPair AbstractPrimerPairModel::primerPairFromIndex(const QModelIndex &index) const
{
    const PrimerPair *primerPair = primerPairPointerFromIndex(index);
    if (primerPair == nullptr)
        return PrimerPair();

    return *primerPair;
}

void AbstractPrimerPairModel::setBioString(const BioString &newBioString)
{
    if (newBioString == bioString_)
        return;

    bioString_ = newBioString;
    emitDataChangedForAllAmpliconLengthCells();
}

bool AbstractPrimerPairModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    PrimerPair *primerPair = primerPairPointerFromIndex(index);
    if (primerPair == nullptr)
        return false;

    if (role != Qt::EditRole)
        return false;

    int primerPairColumn = index.column();
    if (primerPairColumn != eNameColumn)
        return false;

    primerPair->setName(value.toString());
    emit dataChanged(index, index);
    return true;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
PrimerPair *AbstractPrimerPairModel::primerPairPointerFromIndex(const QModelIndex &index)
{
    return primerPairPointerFromRow(index.row());
}

const PrimerPair *AbstractPrimerPairModel::primerPairPointerFromIndex(const QModelIndex &index) const
{
    return primerPairPointerFromRow(index.row());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
QVariant AbstractPrimerPairModel::displayRoleData(const PrimerPair &primerPair, int column) const
{
    switch (column)
    {
    case eNameColumn:
        return primerPair.name();
    case eForwardPrimerSequenceColumn:
        return primerPair.forwardPrimer().sequence().asByteArray();
    case eReversePrimerSequenceColumn:
        return primerPair.reversePrimer().sequence().asByteArray();
    case eCombinedPrimerSequencesColumn:
        return QString("F: %1\nR: %2")
                .arg(QString(primerPair.forwardPrimer().sequence().asByteArray()))
                .arg(QString(primerPair.reversePrimer().sequence().asByteArray()));
    case eForwardTmColumn:
        return ::round(primerPair.forwardPrimer().tm(), 1);
    case eReverseTmColumn:
        return ::round(primerPair.reversePrimer().tm(), 1);
    case eCombinedTmsColumn:
        return QString::number(primerPair.forwardPrimer().tm(), 'f', 1) + "\n" + QString::number(primerPair.reversePrimer().tm(), 'f', 1);
    case eDeltaTmColumn:
        return ::round(primerPair.deltaTm(), 2);
    case eAmpliconLengthColumn:
        return primerPair.longestAmpliconLength(bioString_);
    case eScoreColumn:
        return ::round(primerPair.score(), 2);

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerPairModel::fontRoleData(int column) const
{
    switch (column)
    {
    case eForwardPrimerSequenceColumn:
    case eReversePrimerSequenceColumn:
    case eCombinedPrimerSequencesColumn:
        return QFont("monospace");

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerPairModel::textAlignmentRoleData(int column) const
{
    switch (column)
    {
    case eForwardTmColumn:
    case eReverseTmColumn:
    case eCombinedTmsColumn:
    case eDeltaTmColumn:
    case eAmpliconLengthColumn:
    case eScoreColumn:
        return Qt::AlignCenter;

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerPairModel::verticalHeaderData(int section) const
{
    return QString("%1.").arg(section + 1);
}

QVariant AbstractPrimerPairModel::horizontalHeaderData(int section) const
{
    switch (section)
    {
    case eNameColumn:
        return "Name";
    case eForwardPrimerSequenceColumn:
        return "Forward Primer";
    case eReversePrimerSequenceColumn:
        return "Reverse Primer";
    case eCombinedPrimerSequencesColumn:
        return "Primer Pair";
    case eForwardTmColumn:
        return "Forward Tm (°C)";
    case eReverseTmColumn:
        return "Reverse Tm (°C)";
    case eCombinedTmsColumn:
        return "Pair Tm (°C)";
    case eDeltaTmColumn:
        return QString::fromUtf8("Î” Tm") + QString(" (°C)");
    case eAmpliconLengthColumn:
        return "Amplicon Length (bp)";
    case eScoreColumn:
        return "Score";

    default:
        return QVariant();
    }
}

void AbstractPrimerPairModel::emitDataChangedForAllAmpliconLengthCells()
{
    if (rowCount() == 0)
        return;

    QModelIndex firstAmpliconLengthIndex = index(0, eAmpliconLengthColumn);
    QModelIndex lastAmpliconLengthIndex = index(rowCount() - 1, eAmpliconLengthColumn);
    emit dataChanged(firstAmpliconLengthIndex, lastAmpliconLengthIndex);
}
