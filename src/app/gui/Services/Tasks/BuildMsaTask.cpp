/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QScopedPointer>

#include "BuildMsaTask.h"

#include "../../../core/ObservableMsa.h"
#include "../../../core/Subseq.h"

#include "../../../core/Entities/AminoMsa.h"
#include "../../../core/Entities/DnaMsa.h"

#include "../../../core/Services/AbstractMsaBuilder.h"
#include "../../../core/global.h"
#include "../../../core/macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param taskName [const QString &]
  * @param msaBuilder [AbstractMsaBuilder *]
  * @param sequenceEntities [const QVector<AbstractSeqSPtr> &]
  */
BuildMsaTask::BuildMsaTask(const QString &taskName, AbstractMsaBuilder *msaBuilder, const QVector<AbstractSeqSPtr> &sequenceEntities)
    : IEntityBuilderTask(Ag::Leaf, taskName),
      msaBuilder_(msaBuilder),
      inputSequences_(sequenceEntities)
{
    ASSERT(msaBuilder != nullptr);
    ASSERT(sequenceEntities.size() > 1);

    // Take ownership of msaBuilder
    msaBuilder_->setParent(this);

    connect(msaBuilder_, SIGNAL(alignFinished(QVector<IdBioString>)), SLOT(onAlignFinished(QVector<IdBioString>)));
    connect(msaBuilder_, SIGNAL(progressChanged(qint64,qint64)), SLOT(onMsaBuildProgressChanged(qint64,qint64)));
    connect(msaBuilder_, SIGNAL(error(int,QString)), SLOT(onMsaBuildError(int,QString)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Need to wrap this method in a try/catch block.
  *
  * @param entityName [const QString &]
  * @returns IEntity *
  */
IEntity *BuildMsaTask::createFinalEntity(const QString &entityName) const
{
    if (status() != Ag::Finished)
        return nullptr;

    ASSERT(alignedSequences_.isEmpty() == false);

    Grammar grammar = eUnknownGrammar;
    QScopedPointer<AbstractMsa> msaEntity;
    switch (inputSequences_.first()->type())
    {
    case eAminoSeqNode:
        msaEntity.reset(AminoMsa::createEntity(entityName));
        grammar = eAminoGrammar;
        break;
    case eDnaSeqNode:
        msaEntity.reset(DnaMsa::createEntity(entityName));
        grammar = eDnaGrammar;
        break;

    default:
        return nullptr;
    }

    // Build the underlying MSA object
    QScopedPointer<ObservableMsa> msa(new ObservableMsa(grammar));
    foreach (const IdBioString &alignedSequence, alignedSequences_)
    {
        const AbstractSeqSPtr &abstractSeq = inputSequences_.at(alignedSequence.id_);

        Subseq *subseq = new Subseq(abstractSeq->abstractAnonSeq()->seq_);
        subseq->setBioString(alignedSequence.bioString_);
        subseq->seqEntity_ = abstractSeq;

        if (!msa->append(subseq))
        {
            delete subseq;
            throw QString("Incompatible sequence returned from %1").arg(msaBuilder_->friendlyProgramName());
        }
    }

    msaEntity->setMsa(msa.take());

    return msaEntity.take();

    // ISSUE? Release the references to the internal AbstractSeqSPtr? alignedSequences_?
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void BuildMsaTask::start()
{
    QVector<IdBioString> input;
    input.reserve(inputSequences_.size());
    for (int i=0, z=inputSequences_.size(); i<z; ++i)
        input << IdBioString(i, inputSequences_.at(i)->bioString());

    setStatus(Ag::Running);
    try
    {
        msaBuilder_->align(input);
    }
    catch(QString &errorMessage)
    {
        setStatus(Ag::Error);
        setNote(errorMessage);

        emit error(this);
    }
}

/**
  */
void BuildMsaTask::kill()
{
    msaBuilder_->kill();

    IEntityBuilderTask::kill();
}

/**
  * @param id [int]
  * @param errorMessage [const QString &]
  */
void BuildMsaTask::onMsaBuildError(int /* id */, const QString &errorMessage)
{
    setStatus(Ag::Error);
    setNote(errorMessage);
    emit error(this);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param currentStep [qint64]
  * @param totalSteps [qint64]
  */
void BuildMsaTask::onMsaBuildProgressChanged(qint64 currentStep, qint64 totalSteps)
{
    setProgress(static_cast<double>(currentStep) / static_cast<double>(totalSteps));
}

/**
  * @param alignment [const QVector<IdBioString> &]
  */
void BuildMsaTask::onAlignFinished(const QVector<IdBioString> &alignment)
{
    alignedSequences_ = alignment;

    setStatus(Ag::Finished);
    emit done(this);
}
