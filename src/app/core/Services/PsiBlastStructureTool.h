/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PSIBLASTSTRUCTURETOOL_H
#define PSIBLASTSTRUCTURETOOL_H

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QTemporaryFile>

#include "../BioString.h"
#include "../PODs/Q3Prediction.h"
#include "../util/OptionSet.h"
#include "../global.h"

class NNStructureTool;
class PsiBlastWrapper;

/**
  * PsiBlastStructureTool simplifies predicting secondary structure by integrating PSI-BLAST and the NNStructureTool in
  *  a single connected interface.
  *
  * Depending on the output of PSI-BLAST a couple finish conditions may occur:
  * 1) PSI-BLAST has at least one hit and thus it is possible to produce a secondary structure prediction
  * 2) No PSI-BLAST hits were found. There is no error because the process completed successfully; however, there is no
  *    PSSM data, thus it is not possible to predict the secondary structure - the Q3Prediction is empty.
  */
class PsiBlastStructureTool : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Constructor that will run Psi-Blast with psiBlastConfig and owned by parent
    explicit PsiBlastStructureTool(const QString &psiBlastPath,
                                   const OptionSet &psiBlastOptionSet,
                                   const QString &stage1NeuralNetFile,
                                   const QString &stage2NeuralNetFile,
                                   QObject *parent = nullptr);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isRunning() const;                                         //!< Returns true if the tool is currently running; false otherwise
    void setPsiBlastOptions(const OptionSet &psiBlastOptionSet);    //!< Sets the Psi-Blast configuration to psiBlastConfig

public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void predictSecondary(int id, const BioString &sequence);       //!< Predict the secondary structure of id and sequence
    void cancel();                                                  //!< Cancel the prediction process

Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void canceled(int id);                                          //!< Emitted if the process is cancelled
    void progressChanged(int currentStep, int totalSteps);          //!< Emitted when the progress has completed currentStep out of totalSteps
    void error(int id, const QString &errorMessage);                //!< Emitted when an error is encountered
    void finished(int id, Q3Prediction q3Prediction);               //!< Emitted when the prediction has completed

private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onPsiBlastProgressChanged(int currentIteration, int totalIterations, int currentStep, int totalSteps);
    void onPsiBlastError(int id, const QString &errorMessage);
    void onPsiBlastFinished(int id);

private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QString psiBlastPath_;
    OptionSet psiBlastOptionSet_;
    QString stage1NeuralNetFile_;
    QString stage2NeuralNetFile_;
    PsiBlastWrapper *psiBlastWrapper_;
    NNStructureTool *nnStructureTool_;
    bool running_;
    bool canceled_;
    QScopedPointer<QTemporaryFile> pssmTempFile_;
};

#endif // PSIBLASTSTRUCTURETOOL_H
