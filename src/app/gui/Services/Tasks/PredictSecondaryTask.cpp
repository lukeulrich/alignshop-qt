/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

#include "PredictSecondaryTask.h"
#include "../../../core/constants.h"
#include "../../../core/Adoc.h"
#include "../../../core/Entities/Astring.h"
#include "../../../core/Repositories/AnonSeqRepository.h"

static const QString kStage1NeuralNetFile = "data/nn-sec-stage1.net";
static const QString kStage2NeuralNetFile = "data/nn-sec-stage2.net";

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adoc [Adoc *]
  * @param id [int]
  * @param sequence [const BioString &]
  * @param psiBlastOptions [const OptionSet &]
  * @param name [const QString &]
  */
PredictSecondaryTask::PredictSecondaryTask(Adoc *adoc,
                                           int id,
                                           const BioString &sequence,
                                           const OptionSet &psiBlastOptions,
                                           const QString &name)
    : Task(Ag::Leaf, name),
      adoc_(adoc),
      id_(id),
      sequence_(sequence),
      psiBlastOptions_(psiBlastOptions),
      psiBlastStructureTool_(nullptr)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void PredictSecondaryTask::start()
{
    // TODO: Consolidate this functionality
    QString applicationPath = QCoreApplication::applicationDirPath() + QDir::separator();
    QString psiBlastPath = applicationPath + constants::kPsiBlastRelativePath;
    QString nnStage1File = applicationPath + kStage1NeuralNetFile;
    QString nnStage2File = applicationPath + kStage2NeuralNetFile;

    psiBlastOptions_.set("-num_threads", nThreads());
    if (psiBlastStructureTool_ == nullptr)
    {
        psiBlastStructureTool_ = new PsiBlastStructureTool(psiBlastPath, psiBlastOptions_, nnStage1File, nnStage2File, this);

        connect(psiBlastStructureTool_, SIGNAL(error(int,QString)), this, SLOT(onError(int,QString)));
        connect(psiBlastStructureTool_, SIGNAL(finished(int,Q3Prediction)), this, SLOT(onFinished(int,Q3Prediction)));
        connect(psiBlastStructureTool_, SIGNAL(progressChanged(int,int)), this, SLOT(onProgressChanged(int,int)));
    }

    setStatus(Ag::Running);
    psiBlastStructureTool_->predictSecondary(id_, sequence_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param currentStep [int]
  * @param totalSteps [int]
  */
void PredictSecondaryTask::onProgressChanged(int currentStep, int totalSteps)
{
    setProgress(static_cast<double>(currentStep) / static_cast<double>(totalSteps));
}

/**
  * @param id [int]
  * @param errorMessage [const QString &]
  */
void PredictSecondaryTask::onError(int /* id */, const QString &errorMessage)
{
    setStatus(Ag::Error);
    setNote(errorMessage);

    // OPTIMIZATION: psiBlastStructureTool_->deleteLater();

    emit error(this);
}

/**
  * @param id [int]
  * @param q3 [Q3Prediction]
  */
void PredictSecondaryTask::onFinished(int id, Q3Prediction q3)
{
    setStatus(Ag::Finished);

    if (q3.q3_.isEmpty())
    {
        // This occurs when no significant hits were found to the query and thus no PSSM data could be generated for
        // predicting the secondary structure. In this case, we do not update the secondary structure prediction.
        setNote("No PSI-BLAST hits to query");
        return;
    }

    setNote(q3.q3_.data());

    // OPTIMIZATION: psiBlastStructureTool_->deleteLater();

    // Save the data!
    if (adoc_ != nullptr)
    {
        AstringSPtr astring = adoc_->astringRepository()->find(id);
        if (astring)
        {
            astring->setQ3(q3);
            adoc_->setModified();
        }
    }

    emit done(this);
}
