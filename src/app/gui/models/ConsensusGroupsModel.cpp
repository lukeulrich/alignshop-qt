/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QColor>
#include <QtGui/QFont>

#include "ConsensusGroupsModel.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
ConsensusGroupsModel::ConsensusGroupsModel(QObject *parent)
    : QAbstractTableModel(parent),
      validBioSymbolGroup_(true)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
int ConsensusGroupsModel::columnCount(const QModelIndex & /* parent */) const
{
    return eNumberOfColumns;
}

QVariant ConsensusGroupsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    ASSERT(index.model() == this);

    const BioSymbol &bioSymbol = bioSymbols_.at(index.row());
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case eLabelColumn:
            return bioSymbol.label();
        case eSymbolColumn:
            if (bioSymbol.symbol() != 0)
                return QChar(bioSymbol.symbol());
            return "(Required)";
        case eResiduesColumn:
            if (!bioSymbol.characters().isEmpty())
                return bioSymbol.characters();
            return "(Required)";

        default:
            return QVariant();
        }
    }
    else if (role == Qt::EditRole)
    {
        switch (index.column())
        {
        case eLabelColumn:
            return bioSymbol.label();
        case eSymbolColumn:
            if (bioSymbol.symbol() != 0)
                return QChar(bioSymbol.symbol());
            return QVariant();
        case eResiduesColumn:
            if (!bioSymbol.characters().isEmpty())
                return bioSymbol.characters();
            return QVariant();

        default:
            return QVariant();
        }
    }
    else if (role == Qt::ForegroundRole)
    {
        switch (index.column())
        {
        case eSymbolColumn:
            if (bioSymbol.symbol() == 0 || duplicateSymbolRows_.contains(index.row()))
                return QColor("#aa0000");
            return QVariant();
        case eResiduesColumn:
            if (bioSymbol.characters().isEmpty())
                return QColor("#aa0000");
            return QVariant();

        default:
            return QVariant();
        }
    }
    else if (role == Qt::FontRole)
    {
        switch (index.column())
        {
        case eSymbolColumn:
            if (bioSymbol.symbol() == 0)
            {
                QFont font;
                font.setItalic(true);
                return font;
            }
            return QVariant();

        case eResiduesColumn:
            if (bioSymbol.characters().isEmpty())
            {
                QFont font;
                font.setItalic(true);
                return font;
            }
            return QVariant();

        default:
            return QVariant();
        }
    }

    return QVariant();
}

Qt::ItemFlags ConsensusGroupsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (!index.isValid())
        return flags;

    ASSERT(index.model() == this);

    flags |= Qt::ItemIsEditable;
    return flags;
}

QVariant ConsensusGroupsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return section + 1;

    switch (section)
    {
    case eLabelColumn:
        return "Label";
    case eSymbolColumn:
        return "Symbol";
    case eResiduesColumn:
        return "Amino acids";

    default:
        return QVariant();
    }
}

QModelIndex ConsensusGroupsModel::index(int row, int column, const QModelIndex & /* parent */) const
{
    if (row < 0 || row >= rowCount())
        return QModelIndex();

    if (column < 0 || column >= columnCount())
        return QModelIndex();

    return createIndex(row, column);
}

bool ConsensusGroupsModel::removeRows(int row, int count, const QModelIndex & /* parent */)
{
    if (row < 0 || row >= rowCount())
        return false;

    if (row + count > rowCount())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    bioSymbols_.remove(row, count);
    endRemoveRows();

    updateDuplicateSymbolCells();
    updateBioSymbolGroupValidity();

    return true;
}

int ConsensusGroupsModel::rowCount(const QModelIndex & /* parent */) const
{
    return bioSymbols_.size();
}

bool ConsensusGroupsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role != Qt::EditRole)
        return false;

    ASSERT(index.model() == this);
    BioSymbol &bioSymbol = bioSymbols_[index.row()];
    switch (index.column())
    {
    case eLabelColumn:
        bioSymbol.setLabel(value.toString());
        return true;
    case eSymbolColumn:
        {
            QByteArray newSymbol = value.toByteArray();
            if (!newSymbol.isEmpty())
            {
                bioSymbol.setSymbol(newSymbol.at(0));
            }
            else
            {
                bioSymbol.setSymbol(0);
                setBioSymbolGroupValidity(false);
            }
        }
        updateDuplicateSymbolCells();
        updateBioSymbolGroupValidity();
        return true;
    case eResiduesColumn:
        bioSymbol.setCharacters(value.toByteArray().toUpper());
        if (bioSymbol.characters().isEmpty())
            setBioSymbolGroupValidity(false);
        updateBioSymbolGroupValidity();
        return true;

    default:
        return false;
    }

    return false;
}

BioSymbolGroup ConsensusGroupsModel::bioSymbolGroup() const
{
    return BioSymbolGroup() << bioSymbols_;
}

bool ConsensusGroupsModel::hasDuplicateSymbols() const
{
    return duplicateSymbolRows_.size() > 0;
}

bool ConsensusGroupsModel::hasRowWithEmptySymbol() const
{
    return rowsWithEmptySymbols().size() > 0;
}

bool ConsensusGroupsModel::hasEmptyResidues() const
{
    return rowsWithEmptyResidues().size() > 0;
}

void ConsensusGroupsModel::setBioSymbolGroup(const BioSymbolGroup &newBioSymbolGroup)
{
    beginResetModel();
    bioSymbols_.clear();
    bioSymbols_ = newBioSymbolGroup.bioSymbolVector();
    duplicateSymbolRows_ = QSet<int>::fromList(rowsWithDuplicateSymbols().toList());
    endResetModel();

    updateBioSymbolGroupValidity();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
QModelIndex ConsensusGroupsModel::appendEmptyRow()
{
    int nRows = rowCount();
    beginInsertRows(QModelIndex(), nRows, nRows);
    bioSymbols_ << BioSymbol("New group");
    endInsertRows();

    // Since there is no symbol or characters for this symbol (yet), we know that it is not valid
    setBioSymbolGroupValidity(false);

    return index(nRows, eLabelColumn);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
void ConsensusGroupsModel::setBioSymbolGroupValidity(bool valid)
{
    if (valid == validBioSymbolGroup_)
        return;

    validBioSymbolGroup_ = valid;
    emit bioSymbolGroupValidChanged(validBioSymbolGroup_);
}

QVector<int> ConsensusGroupsModel::rowsWithDuplicateSymbols() const
{
    QSet<char> dupSymbols;
    QHash<char, QVector<int> > symbolRows;
    for (int i=0, z=bioSymbols_.size(); i<z; ++i)
    {
        char symbol = bioSymbols_.at(i).symbol();
        if (symbolRows.contains(symbol))
        {
            symbolRows[symbol].append(i);
            dupSymbols << symbol;
        }
        else
        {
            symbolRows.insert(symbol, QVector<int>() << i);
        }
    }

    QVector<int> rows;
    foreach (const char symbol, dupSymbols)
        rows << symbolRows.value(symbol);

    return rows;
}

QVector<int> ConsensusGroupsModel::rowsWithEmptySymbols() const
{
    QVector<int> rows;
    for (int i=0, z=bioSymbols_.size(); i<z; ++i)
        if (bioSymbols_.at(i).symbol() == 0)
            rows << i;
    return rows;
}

QVector<int> ConsensusGroupsModel::rowsWithEmptyResidues() const
{
    QVector<int> rows;
    for (int i=0, z=bioSymbols_.size(); i<z; ++i)
        if (bioSymbols_.at(i).characters().isEmpty())
            rows << i;
    return rows;
}

void ConsensusGroupsModel::updateBioSymbolGroupValidity()
{
    setBioSymbolGroupValidity(!hasRowWithEmptySymbol() &&
                              !hasDuplicateSymbols() &&
                              !hasEmptyResidues());
}

void ConsensusGroupsModel::updateDuplicateSymbolCells()
{
    QSet<int> oldDupRows = duplicateSymbolRows_;
    QSet<int> newDupRows = QSet<int>::fromList(rowsWithDuplicateSymbols().toList());
    duplicateSymbolRows_ = newDupRows;

    foreach (const int row, oldDupRows)
    {
        if (newDupRows.contains(row))
            continue;

        QModelIndex changedIndex = index(row, eSymbolColumn);
        emit dataChanged(changedIndex, changedIndex);
    }

    foreach (const int row, newDupRows)
    {
        if (oldDupRows.contains(row))
            continue;

        QModelIndex changedIndex = index(row, eSymbolColumn);
        emit dataChanged(changedIndex, changedIndex);
    }
}
