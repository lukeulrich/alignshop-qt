/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONSENSUSGROUPSMODEL_H
#define CONSENSUSGROUPSMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QVector>

#include "../../core/BioSymbol.h"
#include "../../core/BioSymbolGroup.h"
#include "../../core/global.h"

class ConsensusGroupsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        eLabelColumn = 0,
        eSymbolColumn,
        eResiduesColumn,

        eNumberOfColumns
    };

    ConsensusGroupsModel(QObject *parent = nullptr);


    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    BioSymbolGroup bioSymbolGroup() const;
    bool hasDuplicateSymbols() const;
    bool hasRowWithEmptySymbol() const;
    bool hasEmptyResidues() const;
    void setBioSymbolGroup(const BioSymbolGroup &newBioSymbolGroup);


Q_SIGNALS:
    void bioSymbolGroupValidChanged(bool valid);


public Q_SLOTS:
    QModelIndex appendEmptyRow();

private:
    void setBioSymbolGroupValidity(bool valid);
    QVector<int> rowsWithDuplicateSymbols() const;
    QVector<int> rowsWithEmptySymbols() const;
    QVector<int> rowsWithEmptyResidues() const;
    void updateBioSymbolGroupValidity();
    void updateDuplicateSymbolCells();

    QVector<BioSymbol> bioSymbols_;
    bool validBioSymbolGroup_;
    QSet<int> duplicateSymbolRows_;
};

#endif // CONSENSUSGROUPSMODEL_H
