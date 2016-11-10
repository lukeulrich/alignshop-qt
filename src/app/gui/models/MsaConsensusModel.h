/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSACONSENSUSMODEL_H
#define MSACONSENSUSMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QVector>
#include "../../core/BioSymbolGroup.h"
#include "../../core/global.h"

class ClosedIntRange;
class LiveMsaCharCountDistribution;
class LiveSymbolString;

/**
  * Models a collection of consensus items with regard to a particular LiveMsaCharCountDistribution
  */
class MsaConsensusModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        eThresholdColumn = 0,
        eFriendlyThresholdColumn,
        eSymbolStringColumn,

        eNumberOfColumns
    };


    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit MsaConsensusModel(QObject *parent = nullptr);
    ~MsaConsensusModel();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void setLiveMsaCharCountDistribution(LiveMsaCharCountDistribution *liveMsaCharCountDistribution);
    int symbolStringLength() const;
    QVector<double> thresholds() const;


public Q_SLOTS:
    void clear();
    void setThresholds(const QVector<double> &newThresholds);
    void setConsensusSymbolGroupPrototype(const BioSymbolGroup &consensusSymbolGroup);


private Q_SLOTS:
    void sendDataChangedSignalForSender();


private:
    struct ConsensusItem
    {
        double threshold_;
        LiveSymbolString *liveSymbolString_;

        ConsensusItem();
    };

    QVariant displayRoleData(const ConsensusItem &consensusItem, const int column) const;
    QVariant editRoleData(const ConsensusItem &consensusItem, const int column) const;

    bool isValidThreshold(const double threshold) const;
    bool isValidColumnNumber(int column) const;
    bool isValidRowNumber(int row) const;
    ConsensusItem makeConsensusItemForThreshold(const double threshold) const;
    LiveSymbolString *makeSymbolStringForThreshold(const double threshold) const;
    void observeLiveSymbolString(LiveSymbolString *liveSymbolString);
    void rebuildSymbolStrings();
    void updateRowThreshold(const int row, const double threshold);
    QModelIndex symbolStringIndexFromRow(const int row) const;
    QModelIndex indexForSymbolString(const LiveSymbolString *liveSymbolString) const;
    void clearConsensusItems();

    LiveMsaCharCountDistribution *liveMsaCharCountDistribution_;
    QVector<ConsensusItem> consensusItems_;
    BioSymbolGroup consensusSymbolGroupPrototype_;
};

#endif // MSACONSENSUSMODEL_H
