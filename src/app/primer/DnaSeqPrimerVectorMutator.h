/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNASEQPRIMERVECTORMUTATOR_H
#define DNASEQPRIMERVECTORMUTATOR_H

#include <QtCore/QVector>

#include "PrimerPair.h"
#include "../core/Entities/DnaSeq.h"
#include "../core/global.h"

class ClosedIntRange;

class DnaSeqPrimerVectorMutator : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    DnaSeqPrimerVectorMutator(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    ClosedIntRange appendPrimers(DnaSeqSPtr &dnaSeq, const PrimerVector &primerVector) const;
    void insertPrimers(DnaSeqSPtr &dnaSeq, int row, const PrimerVector &primerVector) const;
    void removePrimers(DnaSeqSPtr &dnaSeq, const ClosedIntRange &primerRowRange) const;


Q_SIGNALS:
    void primersAboutToBeAppended(const DnaSeqSPtr &dnaSeq, int startRow, int endRow) const;
    void primersAppended(const DnaSeqSPtr &dnaSeq, int startRow, int endRow) const;
    void primersAboutToBeInserted(const DnaSeqSPtr &dnaSeq, int startRow, int endRow) const;
    void primersInserted(const DnaSeqSPtr &dnaSeq, int startRow, int endRow) const;
    void primersAboutToBeRemoved(const DnaSeqSPtr &dnaSeq, int startRow, int endRow) const;
    void primersRemoved(const DnaSeqSPtr &dnaSeq, int startRow, int endRow) const;
};

#endif // DNASEQPRIMERVECTORMUTATOR_H
