/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTPRIMERMODEL_H
#define ABSTRACTPRIMERMODEL_H

#include <QtCore/QAbstractTableModel>

#include "Primer.h"
#include "../core/BioString.h"
#include "../core/global.h"

class IPrimerMutator;

class AbstractPrimerModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        eNameColumn = 0,
        eRestrictionEnzymeNameColumn,
        eRestrictionEnzymeSequenceColumn,
        eCoreSequenceColumn,
        eSequenceColumn,
        eTmColumn,
        eStrandColumn,
        eMessageColumn,

        eNumberOfColumns
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor
    AbstractPrimerModel(IPrimerMutator *primerMutator, QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioString bioString() const;
    virtual void clear() = 0;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool indexHasPrimerSearchParameters(const QModelIndex &index) const;
    bool isValidRow(const int row) const;
    virtual PrimerVector primers() const = 0;
    Primer primerFromIndex(const QModelIndex &index) const;
    IPrimerMutator *primerMutator() const;
    void setBioString(const BioString &newBioString);
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    // Custom member methods and convenience methods
    int primerId(const int row) const;
    Primer primerFromRow(const int row) const;


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    Primer *primerPointerFromIndex(const QModelIndex &index);
    const Primer *primerPointerFromIndex(const QModelIndex &index) const;
    virtual Primer *primerPointerFromRow(const int row) = 0;
    virtual const Primer *primerPointerFromRow(const int row) const = 0;


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onPrimerMutatorNameChanged(int primerId);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void observePrimerMutator();
    QVariant displayRoleData(const Primer &primer, int column) const;
    QVariant editRoleData(const Primer &primer, int column) const;
    QVariant fontRoleData(int column) const;
    QVariant textAlignmentRole(int column) const;
    QVariant verticalHeaderDisplayRoleData(int section) const;
    QVariant horizontalHeaderDisplayRoleData(int section) const;
    QVariant horizontalHeaderToolTipRoleData(int section) const;
    QString primerStrand(const Primer &primer) const;



    // ------------------------------------------------------------------------------------------------
    // Private members
    IPrimerMutator *primerMutator_;
    BioString bioString_;
};

#endif // ABSTRACTPRIMERMODEL_H
