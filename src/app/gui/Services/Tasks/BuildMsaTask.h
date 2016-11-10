/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BUILDMSATASK_H
#define BUILDMSATASK_H

#include <QtCore/QVector>

#include "IEntityBuilderTask.h"
#include "../../../core/PODs/IdBioString.h"
#include "../../../core/Entities/AbstractSeq.h"

class AbstractMsa;
class AbstractMsaBuilder;

class BuildMsaTask : public IEntityBuilderTask
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    // Takes ownership of msaBuilder
    BuildMsaTask(const QString &taskName, AbstractMsaBuilder *msaBuilder, const QVector<AbstractSeqSPtr> &sequenceEntities);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    IEntity *createFinalEntity(const QString &entityName) const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void start();
    void kill();


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onMsaBuildError(int id, const QString &errorMessage);
    void onMsaBuildProgressChanged(qint64 currentStep, qint64 totalSteps);
    void onAlignFinished(const QVector<IdBioString> &alignment);


private:
    AbstractMsaBuilder *msaBuilder_;
    QVector<AbstractSeqSPtr> inputSequences_;
    QVector<IdBioString> alignedSequences_;
};

#endif // BUILDMSATASK_H
