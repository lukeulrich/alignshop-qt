/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PSIBLASTSERVICE_H
#define PSIBLASTSERVICE_H

#include "AbstractProcessWrapper.h"

#include "../BioString.h"
#include "../global.h"

/**
  * PsiBlastWrapper conveniently wraps the execution of PSI-BLAST+ and when the custom AG-PSI-BLAST is used (expected),
  * provides progress signals.
  *
  * The default version of PSI-BLAST+ from NCBI fails to meet two of our goals:
  * 1) No mechanism for obtaining the BLAST search progress
  * 2) No mechanism for outputting a high-resolution ASCII PSSM matrix
  *
  * The original blast+ source code has been modified to provide this functionality and these changes are stored within
  * blast+ subdirectory beneath the root AlignShop directory.
  *
  * PsiBlastWrapper facilitates the execution of PSI-BLAST along with reporting progress updates and retrieval of the
  * raw Psi-Blast output.
  */
class PsiBlastWrapper : public AbstractProcessWrapper
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct instance with config and parent
    PsiBlastWrapper(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QString friendlyProgramName() const;                //!< Returns PSI-BLAST
    virtual int id() const;                             //!< Returns the id for the current process


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void psiblast(int id, const BioString &bioString);  //!< Start the PSI-BLAST process with id and the bioString sequence


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    //! Emitted whenever the progress has changed
    void progressChanged(int currentIteration, int totalIterations, int currentStep, int totalSteps);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onReadyReadStandardError();                    //!< Called when there is data available on standard error (e.g. progress messages)
    void onProcessStarted();                            //!< Called when the process has started


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! Utility method for extracting a pair of numbers (e.g. iteration or progress lines) from a string
    QPair<int, int> extractNumberPair(const char *x) const;
    void reset();                                       //!< Clears the internal process variables to their default state; does not clear the PSI-BLAST configuration

    // ------------------------------------------------------------------------------------------------
    // Private members
    int currentIteration_;              //!< The current iteration; local variable to remember between events
    int totalIterations_;               //!< The total number of iterations; local variable to remember between events
    int id_;                            //!< Current sequence id being psi-blasted
    BioString sequence_;                //!< Current sequence string being psi-blasted
};

#endif // PSIBLASTSERVICE_H
