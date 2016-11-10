/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNASEQPRIMERPAIRMODEL_H
#define DNASEQPRIMERPAIRMODEL_H

#include "AbstractPrimerModel.h"
#include "../core/Entities/DnaSeq.h"

class DnaSeqPrimerVectorMutator;

class DnaSeqPrimerModel : public AbstractPrimerModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    DnaSeqPrimerModel(DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator, IPrimerMutator *primerMutator, QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void clear();
    DnaSeqSPtr dnaSeq() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    PrimerVector primers() const;
    void setDnaSeq(const DnaSeqSPtr &dnaSeq);


Q_SIGNALS:
    void dnaSeqChanged(const DnaSeqSPtr &newDnaSeq);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual Primer *primerPointerFromRow(const int row);
    virtual const Primer *primerPointerFromRow(const int row) const;


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onDnaSeqPrimersAboutToBeAppended(const DnaSeqSPtr &dnaSeq, int startRow, int endRow);
    void onDnaSeqPrimersAppended(const DnaSeqSPtr &dnaSeq, int startRow, int endRow);
    void onDnaSeqPrimersAboutToBeInserted(const DnaSeqSPtr &dnaSeq, int startRow, int endRow);
    void onDnaSeqPrimersInserted(const DnaSeqSPtr &dnaSeq, int startRow, int endRow);
    void onDnaSeqPrimersAboutToBeRemoved(const DnaSeqSPtr &dnaSeq, int startRow, int endRow);
    void onDnaSeqPrimersRemoved(const DnaSeqSPtr &dnaSeq, int startRow, int endRow);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void observePrimerVectorMutator();


    // ------------------------------------------------------------------------------------------------
    // Private members
    DnaSeqPrimerVectorMutator *dnaSeqPrimerVectorMutator_;
    DnaSeqSPtr dnaSeq_;
};

#endif // DNASEQPRIMERMODEL_H
