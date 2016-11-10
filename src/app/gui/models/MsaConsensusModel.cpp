/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFont>

#include "MsaConsensusModel.h"
#include "../../core/data/CommonBioSymbolGroups.h"
#include "../../core/Services/SymbolStringCalculator.h"
#include "../../core/LiveMsaCharCountDistribution.h"
#include "../../core/LiveSymbolString.h"
#include "../../core/macros.h"
#include "../../core/misc.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
MsaConsensusModel::ConsensusItem::ConsensusItem()
    : threshold_(0.),
      liveSymbolString_(nullptr)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
MsaConsensusModel::MsaConsensusModel(QObject *parent)
    : QAbstractTableModel(parent),
      liveMsaCharCountDistribution_(nullptr),
      consensusSymbolGroupPrototype_(constants::CommonBioSymbolGroups::defaultConsensusSymbolGroup())
{
}

MsaConsensusModel::~MsaConsensusModel()
{
    clearConsensusItems();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
int MsaConsensusModel::columnCount(const QModelIndex & /* parent */) const
{
    return eNumberOfColumns;
}

QVariant MsaConsensusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ASSERT(isValidRowNumber(index.row()));
    ASSERT(isValidColumnNumber(index.column()));
    const ConsensusItem &item = consensusItems_.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:           return displayRoleData(item, index.column());
    case Qt::EditRole:              return editRoleData(item, index.column());

    default:
        return QVariant();
    }
}

Qt::ItemFlags MsaConsensusModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    ASSERT(isValidRowNumber(index.row()));
    ASSERT(isValidColumnNumber(index.column()));
    Qt::ItemFlags itemFlags = QAbstractTableModel::flags(index);
    if (index.column() == eFriendlyThresholdColumn)
        itemFlags |= Qt::ItemIsEditable;
    return itemFlags;
}

QVariant MsaConsensusModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
            case eFriendlyThresholdColumn:
            case eThresholdColumn:
                return "Threshold";
            case eSymbolStringColumn:
                return "Consensus";

            default:
                return QVariant();
            }
        }
    }
    else
    {
        if (role == Qt::DisplayRole)
            return section + 1;
    }

    return QVariant();
}

QModelIndex MsaConsensusModel::index(int row, int column, const QModelIndex & /* parent */) const
{
    if (liveMsaCharCountDistribution_ == nullptr)
        return QModelIndex();

    if (!isValidRowNumber(row) || !isValidColumnNumber(column))
        return QModelIndex();

    return createIndex(row, column);
}

int MsaConsensusModel::rowCount(const QModelIndex & /* parent */) const
{
    return consensusItems_.size();
}

bool MsaConsensusModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (index.column() != eFriendlyThresholdColumn)
        return false;

    bool ok;
    int thresholdPercentage = value.toInt(&ok);
    if (!ok)
        return false;

    if (thresholdPercentage < 50 || thresholdPercentage > 100)
        return false;

    updateRowThreshold(index.row(), thresholdPercentage / 100.);

    return true;
}

void MsaConsensusModel::setLiveMsaCharCountDistribution(LiveMsaCharCountDistribution *liveMsaCharCountDistribution)
{
    if (liveMsaCharCountDistribution_ == liveMsaCharCountDistribution)
        return;

    liveMsaCharCountDistribution_ = liveMsaCharCountDistribution;
    rebuildSymbolStrings();
}

int MsaConsensusModel::symbolStringLength() const
{
    if (liveMsaCharCountDistribution_ == nullptr)
        return 0;

    return liveMsaCharCountDistribution_->length();
}

QVector<double> MsaConsensusModel::thresholds() const
{
    QVector<double> doubles;
    for (int i=0, z=rowCount(); i<z; ++i)
        doubles << consensusItems_.at(i).threshold_;
    return doubles;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
void MsaConsensusModel::clear()
{
    setThresholds(QVector<double>());
}

void MsaConsensusModel::setThresholds(const QVector<double> &newThresholds)
{
    beginResetModel();
    clearConsensusItems();
    foreach (const double threshold, newThresholds)
    {
        if (!isValidThreshold(threshold))
            continue;

        consensusItems_ << makeConsensusItemForThreshold(threshold);
        observeLiveSymbolString(consensusItems_.last().liveSymbolString_);
    }
    endResetModel();
}

void MsaConsensusModel::setConsensusSymbolGroupPrototype(const BioSymbolGroup &consensusSymbolGroup)
{
    consensusSymbolGroupPrototype_ = consensusSymbolGroup;
    rebuildSymbolStrings();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
void MsaConsensusModel::sendDataChangedSignalForSender()
{
    ASSERT(qobject_cast<LiveSymbolString *>(sender()) != 0);
    LiveSymbolString *sourceSymbolString = static_cast<LiveSymbolString *>(sender());
    QModelIndex symbolStringIndex = indexForSymbolString(sourceSymbolString);
    ASSERT(symbolStringIndex.isValid());
    emit dataChanged(symbolStringIndex, symbolStringIndex);
}

QVariant MsaConsensusModel::displayRoleData(const ConsensusItem &consensusItem, const int column) const
{
    switch (column)
    {
    case eThresholdColumn:
        return consensusItem.threshold_;
    case eFriendlyThresholdColumn:
        return QString("Consensus: %1%").arg(static_cast<int>(consensusItem.threshold_ * 100));
    case eSymbolStringColumn:
        if (consensusItem.liveSymbolString_ == nullptr)
            return QByteArray();
        return consensusItem.liveSymbolString_->symbolString();

    default:
        return QVariant();
    }
}

QVariant MsaConsensusModel::editRoleData(const ConsensusItem &consensusItem, const int column) const
{
    switch (column)
    {
    case eThresholdColumn:
    case eFriendlyThresholdColumn:
        return static_cast<int>(consensusItem.threshold_ * 100.);

    default:
        return QVariant();
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
bool MsaConsensusModel::isValidThreshold(const double threshold) const
{
    return threshold >= .5 && threshold <= 1.;
}

bool MsaConsensusModel::isValidColumnNumber(int column) const
{
    return column >= 0 && column < columnCount();
}

bool MsaConsensusModel::isValidRowNumber(int row) const
{
    return row >= 0 && row < rowCount();
}

MsaConsensusModel::ConsensusItem MsaConsensusModel::makeConsensusItemForThreshold(const double threshold) const
{
    ConsensusItem consensusItem;
    consensusItem.threshold_ = threshold;
    consensusItem.liveSymbolString_ = makeSymbolStringForThreshold(threshold);
    return consensusItem;
}

LiveSymbolString *MsaConsensusModel::makeSymbolStringForThreshold(const double threshold) const
{
    if (liveMsaCharCountDistribution_ == nullptr)
        return nullptr;

    using namespace constants::CommonBioSymbolGroups;
    BioSymbolGroup consensusGroup = consensusSymbolGroupPrototype_;
    consensusGroup.setThresholdForAllBioSymbols(threshold);
    SymbolStringCalculator calculator(consensusGroup, kDefaultConsensusSymbol);
    return new LiveSymbolString(liveMsaCharCountDistribution_, calculator);
}

void MsaConsensusModel::observeLiveSymbolString(LiveSymbolString *liveSymbolString)
{
    if (liveSymbolString == nullptr)
        return;

    connect(liveSymbolString, SIGNAL(dataChanged(ClosedIntRange)), SLOT(sendDataChangedSignalForSender()));
    connect(liveSymbolString, SIGNAL(symbolsInserted(ClosedIntRange)), SLOT(sendDataChangedSignalForSender()));
    connect(liveSymbolString, SIGNAL(symbolsRemoved(ClosedIntRange)), SLOT(sendDataChangedSignalForSender()));
}

void MsaConsensusModel::rebuildSymbolStrings()
{
    for (int i=0, z=rowCount(); i<z; ++i)
    {
        ConsensusItem &consensusItem = consensusItems_[i];
        delete consensusItem.liveSymbolString_;
        consensusItem.liveSymbolString_ = makeSymbolStringForThreshold(consensusItem.threshold_);
        observeLiveSymbolString(consensusItem.liveSymbolString_);
        QModelIndex symbolStringIndex = symbolStringIndexFromRow(i);
        emit dataChanged(symbolStringIndex, symbolStringIndex);
    }
}

void MsaConsensusModel::updateRowThreshold(const int row, const double threshold)
{
    ASSERT(isValidRowNumber(row));
    ConsensusItem &consensusItem = consensusItems_[row];
    if (qFuzzyCompare(consensusItem.threshold_, threshold))
        return;

    delete consensusItem.liveSymbolString_;
    consensusItem.threshold_ = threshold;
    consensusItem.liveSymbolString_ = makeSymbolStringForThreshold(threshold);

    // All three columns have changed
    QModelIndex thresholdIndex = index(row, eThresholdColumn);
    emit dataChanged(thresholdIndex, thresholdIndex);
    QModelIndex friendlyThresholdIndex = index(row, eFriendlyThresholdColumn);
    emit dataChanged(friendlyThresholdIndex, friendlyThresholdIndex);
    QModelIndex symbolStringIndex = symbolStringIndexFromRow(row);
    emit dataChanged(symbolStringIndex, symbolStringIndex);
}

QModelIndex MsaConsensusModel::symbolStringIndexFromRow(const int row) const
{
    return index(row, eSymbolStringColumn);
}

QModelIndex MsaConsensusModel::indexForSymbolString(const LiveSymbolString *liveSymbolString) const
{
    if (liveSymbolString == nullptr)
        return QModelIndex();

    for (int i=0, z=rowCount(); i<z; ++i)
    {
        const ConsensusItem &consensusItem = consensusItems_[i];
        if (consensusItem.liveSymbolString_ != liveSymbolString)
            continue;

        return symbolStringIndexFromRow(i);
    }

    return QModelIndex();
}

void MsaConsensusModel::clearConsensusItems()
{
    foreach (const ConsensusItem &consensusItem, consensusItems_)
        delete consensusItem.liveSymbolString_;
    consensusItems_.clear();
}
