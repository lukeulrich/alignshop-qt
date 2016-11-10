/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <cstdio>              // For sscanf and sscanf_s

#include "PsiBlastWrapper.h"
#include "../constants/PsiBlastConstants.h"
#include "../macros.h"
#include "../misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
PsiBlastWrapper::PsiBlastWrapper(QObject *parent)
    : AbstractProcessWrapper(parent),
      currentIteration_(0),
      totalIterations_(0),
      id_(0)
{
    setOptionProfile(constants::PsiBlast::kPsiBlastOptionProfile);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString PsiBlastWrapper::friendlyProgramName() const
{
    return constants::PsiBlast::kFriendlyPsiBlastName;
}

/**
  * @returns int
  */
int PsiBlastWrapper::id() const
{
    return id_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param id [int]
  * @param bioString [const BioString &]
  */
void PsiBlastWrapper::psiblast(int id, const BioString &bioString)
{
    if (bioString.isEmpty())
    {
        emit error(id, "Empty biostring");
        return;
    }

    reset();

    id_ = id;
    sequence_ = bioString;

    AbstractProcessWrapper::execute();
    // From here on, all further interaction is handled via signals/slots (which are setup in the constructor).
    // Specifically, the onProcessStarted method will be called, which then passes the sequence to PsiBlast via its
    // stdin.
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * All progress messages are dumped to stderr and this method transforms those lines into progress signals.
  * Specifically, the custom AG Psi-Blast executable outputs the current and total iterations with the line:
  *
  * @iteration: {current iteration}, {total iterations}
  *
  * Similarly, progress messages take the form:
  *
  * @progress: {current step}, {total step}
  */
void PsiBlastWrapper::onReadyReadStandardError()
{
    process_->setReadChannel(QProcess::StandardError);
    while (process_->canReadLine())
    {
        QByteArray line = process_->readLine();

        // Now check for progress messages
        if (line.startsWith("@progress: "))
        {
            // 11 is the length of '@progress: '
            QPair<int, int> numberPair = extractNumberPair(line.constData() + 11);
            if (numberPair.first && numberPair.second)
                emit progressChanged(currentIteration_, totalIterations_, numberPair.first, numberPair.second);
        }
        else if (line.startsWith("@iteration: "))
        {
            // 12 is the length of '@iteration: '
            QPair<int, int> numberPair = extractNumberPair(line.constData() + 12);
            if (numberPair.first && numberPair.second)
            {
                currentIteration_ = numberPair.first;
                totalIterations_ = numberPair.second;
            }
        }
        else
        {
            errorOutput_.append(line);
        }
    }
}

/**
  * Rather than create a separate file containing the sequence and pass this information to Psi-Blast as a command line
  * argument, we provide the query sequence via STDIN. This method checks submits this information to Psi-Blast in the
  * FASTA format and then closes the stdin stream. Otherwise, Psi-Blast would simply hang waiting for input.
  */
void PsiBlastWrapper::onProcessStarted()
{
    ASSERT(sequence_.isEmpty() == false);
    ASSERT(isRunning());

    // Now that the process has successfully begun, send the sequence to the process by writing to its standard input.
    // TODO: Check that all data was actually written!
    writeAll(*process_, ">");
    writeAll(*process_, QByteArray::number(id_));
    writeAll(*process_, "\n");
    writeAll(*process_, sequence_.asByteArray());

    // Once we close the write channel (ie. stdin), psiblast should initiate the search process (assuming a valid
    // configuration).
    process_->closeWriteChannel();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param x [const char *]
  * @returns QPair<int, int>
  */
QPair<int, int> PsiBlastWrapper::extractNumberPair(const char *x) const
{
    ASSERT(x != nullptr);

    QPair<int, int> numberPair;
    int nFound = sscanf(x, "%d, %d", &numberPair.first, &numberPair.second);
    if (nFound == 2)
        return numberPair;

    // Return an empty pair
    return qMakePair(0, 0);
}

/**
  */
void PsiBlastWrapper::reset()
{
    AbstractProcessWrapper::reset();

    currentIteration_ = 0;
    totalIterations_ = 0;
    id_ = 0;
    sequence_.clear();
}
