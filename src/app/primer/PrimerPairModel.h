/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERPAIRMODEL_H
#define PRIMERPAIRMODEL_H

#include <QtCore/QVector>

#include "AbstractPrimerPairModel.h"

class PrimerPairModel : public AbstractPrimerPairModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    PrimerPairModel(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual void clear();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual PrimerPairVector primerPairs() const;
    void setPrimerPairs(const PrimerPairVector &newPrimerPairs);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual PrimerPair *primerPairPointerFromRow(const int row);
    virtual const PrimerPair *primerPairPointerFromRow(const int row) const;


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    PrimerPairVector primerPairs_;
};

#endif // PRIMERPAIRMODEL_H
