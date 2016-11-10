/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTPRIMERPAIRMODEL_H
#define ABSTRACTPRIMERPAIRMODEL_H

#include <QtCore/QAbstractTableModel>
#include "PrimerPair.h"
#include "../core/BioString.h"
#include "../core/global.h"

class AbstractPrimerPairModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        eNameColumn = 0,
        eForwardPrimerSequenceColumn,
        eReversePrimerSequenceColumn,
        eCombinedPrimerSequencesColumn,
        eForwardTmColumn,
        eReverseTmColumn,
        eCombinedTmsColumn,
        eDeltaTmColumn,
        eAmpliconLengthColumn,
        eScoreColumn,

        eNumberOfColumns
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor
    AbstractPrimerPairModel(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioString bioString() const;
    virtual void clear() = 0;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool isValidRow(const int row) const;
    virtual PrimerPairVector primerPairs() const = 0;
    PrimerPair primerPairFromIndex(const QModelIndex &index) const;
    void setBioString(const BioString &newBioString);
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    PrimerPair *primerPairPointerFromIndex(const QModelIndex &index);
    const PrimerPair *primerPairPointerFromIndex(const QModelIndex &index) const;
    virtual PrimerPair *primerPairPointerFromRow(const int row) = 0;
    virtual const PrimerPair *primerPairPointerFromRow(const int row) const = 0;


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void observePrimerPairMutator();
    QVariant displayRoleData(const PrimerPair &primerPair, int column) const;
    QVariant fontRoleData(int column) const;
    QVariant textAlignmentRoleData(int column) const;
    QVariant verticalHeaderData(int section) const;
    QVariant horizontalHeaderData(int section) const;
    void emitDataChangedForAllAmpliconLengthCells();



    // ------------------------------------------------------------------------------------------------
    // Private members
    BioString bioString_;
};

#endif // ABSTRACTPRIMERPAIRMODEL_H
