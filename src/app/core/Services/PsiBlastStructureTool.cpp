/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "PsiBlastStructureTool.h"

#include "PsiBlastWrapper.h"
#include "NNStructureTool.h"

#include "../Parsers/PssmParser.h"
#include "../PODs/Pssm.h"
#include "../PODs/NormalizedPssm.h"
#include "../constants/PsiBlastConstants.h"
#include "../util/PssmUtil.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param psiBlastPath [const QString &]
  * @param psiBlastOptionSet [const OptionSet &]
  * @param stage1NeuralNetFile [const QString &]
  * @param stage2NeuralNetFile [const QString &]
  * @param parent [QObject *]
  */
PsiBlastStructureTool::PsiBlastStructureTool(const QString &psiBlastPath,
                                             const OptionSet &psiBlastOptionSet,
                                             const QString &stage1NeuralNetFile,
                                             const QString &stage2NeuralNetFile, QObject *parent)
    : QObject(parent),
      psiBlastPath_(psiBlastPath),
      stage1NeuralNetFile_(stage1NeuralNetFile),
      stage2NeuralNetFile_(stage2NeuralNetFile),
      psiBlastWrapper_(nullptr),
      nnStructureTool_(nullptr),
      running_(false),
      canceled_(false)
{
    setPsiBlastOptions(psiBlastOptionSet);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool PsiBlastStructureTool::isRunning() const
{
    return running_;
}

/**
  * @param psiBlastOptionSet [const OptionSet &]
  */
void PsiBlastStructureTool::setPsiBlastOptions(const OptionSet &psiBlastOptionSet)
{
    using namespace constants::PsiBlast;
    ASSERT_X(psiBlastOptionSet.contains(kIterationsOpt) &&
             psiBlastOptionSet.value(kIterationsOpt).toInt() > 1,
             "Number of PSI-BLAST iterations must be at least 2 when predicting secondary structure");

    psiBlastOptionSet_ = psiBlastOptionSet;

    // Since only the AG-PSSM is only required for successful operation, all other output is unnecessary. Thus, its
    // output is reduced to the minimum.
    //
    // Note: Different output formats have different minimum values for the number of descriptions and alignments. For
    // example, CSVOutput requires at least 1 description and 1 alignment; however, pairwise only requires one alignment
    // or 1 description.
    psiBlastOptionSet_.set(kOutputFormatOpt, eCSVOutput);
    psiBlastOptionSet_.set(kNumAlignmentsOpt, 1);
    psiBlastOptionSet_.set(kNumDescriptionsOpt, 1);

    // No need to assign the psiblast config to the psiblast service at this point, because it must be assigned
    // every time predictSecondary is called to accommodate the unique temporary file name for the PSSM.
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param id [int]
  * @param sequence [const BioString &]
  */
void PsiBlastStructureTool::predictSecondary(int id, const BioString &sequence)
{
    ASSERT(running_ == false);
    if (running_)
        return;

    canceled_ = false;

    // Check that the configuration is appropriate
    using namespace constants::PsiBlast;
    if (!psiBlastOptionSet_.contains(kIterationsOpt) ||
        psiBlastOptionSet_.value(kIterationsOpt).toInt() < 2)
    {
        emit error(id, "Secondary prediction requires Psi-Blast to run at least 2 iterations");
        return;
    }

    // Lazy creation of the psiblast service and NN structure tool
    if (psiBlastWrapper_ == nullptr)
    {
        psiBlastWrapper_ = new PsiBlastWrapper(this);
        psiBlastWrapper_->setProgram(psiBlastPath_);

        connect(psiBlastWrapper_, SIGNAL(error(int,QString)), SLOT(onPsiBlastError(int,QString)));
        connect(psiBlastWrapper_, SIGNAL(finished(int,QByteArray)), SLOT(onPsiBlastFinished(int)));
        connect(psiBlastWrapper_, SIGNAL(progressChanged(int,int,int,int)), SLOT(onPsiBlastProgressChanged(int,int,int,int)));
    }

    if (nnStructureTool_ == nullptr)
    {
        try
        {
            nnStructureTool_ = new NNStructureTool(stage1NeuralNetFile_, stage2NeuralNetFile_, this);
        }
        catch (QString &errorMessage)
        {
            Q_UNUSED(errorMessage);
            emit error(id, QString("Unable to initialize neural network: %1").arg(errorMessage));
            return;
        }
    }

    // Configure the out file for saving the PSSM data
    ASSERT(pssmTempFile_.isNull());
    QScopedPointer<QTemporaryFile> pssmTempFile(new QTemporaryFile(QDir::tempPath() + "/psiblast-pssm.XXXXXX"));
    if (!pssmTempFile->open())
    {
        emit error(id, QString("Unable to create temporary file for psiblast pssm"));
        return;
    }
    psiBlastOptionSet_.set(kAgPssmFileOpt, pssmTempFile->fileName());
    if (!psiBlastWrapper_->setOptions(psiBlastOptionSet_))
    {
        emit error(id, "Error setting Psi-Blast options");
        return;
    }

    // Kick off the PSI-BLAST request
    try
    {
        psiBlastWrapper_->psiblast(id, sequence);
    }
    catch (const QString &errorMessage)
    {
        emit error(id, errorMessage);
        return;
    }

    running_ = true;
    pssmTempFile_.reset(pssmTempFile.take());
}

/**
  * Only permit canceling during the psiblast stage. There is no canceling during the neural network phase.
  */
void PsiBlastStructureTool::cancel()
{
    if (!isRunning())
        return;

    canceled_ = true;
    psiBlastWrapper_->kill();

    // After calling kill, the process should then error out which will trigger onPsiBlastError and accordingly update
    // this tool's running state.
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param currentIteration [int]
  * @param totalIterations [int]
  * @param currentStep [int]
  * @param totalSteps [int]
  */
void PsiBlastStructureTool::onPsiBlastProgressChanged(int currentIteration,
                                                         int totalIterations,
                                                         int currentStep,
                                                         int totalSteps)
{
    emit progressChanged( (currentIteration - 1) * totalSteps + currentStep,
                          totalIterations * totalSteps);
}

/**
  * @param id [int]
  * @param errorMessage [const QString &]
  */
void PsiBlastStructureTool::onPsiBlastError(int id, const QString &errorMessage)
{
    pssmTempFile_.reset();
    running_ = false;
    if (canceled_)
        emit canceled(id);
    else
        emit error(id, errorMessage);
}

/**
  * @param id [int]
  */
void PsiBlastStructureTool::onPsiBlastFinished(int id)
{
    // If the temp file is empty that means that no hits were found -> no PSSM data is available -> cannot predict
    // secondary structure. This does not constitute an error condition though.
    if (QFileInfo(pssmTempFile_->fileName()).size() == 0)
    {
        running_ = false;
        pssmTempFile_.reset();

        // It is also possible that not all iterations were performed, and thus the final progress changed might not
        // reflect a completed status.
        emit progressChanged(1, 1);

        emit finished(id, Q3Prediction());
        return;
    }

    try
    {
        // Parse the resultant pssm file
        PssmParser parser;
        Pssm pssm = parser.parseFile(pssmTempFile_->fileName());

        pssmTempFile_.reset();

        // Normalize the PSSM
        NormalizedPssm normalizedPssm = ::normalizePssm(pssm);

        // Execute the neural network tool
        Q3Prediction q3prediction = nnStructureTool_->predictSecondary(normalizedPssm);

        // Send out the result via a signal
        emit finished(id, q3prediction);
    }
    catch (const QString &errorMessage)
    {
        emit error(id, errorMessage);
    }

    running_ = false;
}
