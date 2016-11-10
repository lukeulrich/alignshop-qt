/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTTASK_H
#define BLASTTASK_H

#include <QtCore/QDir>

#include "IEntityBuilderTask.h"

#include "../../../core/BioString.h"
#include "../../../core/PODs/BlastDatabaseSpec.h"
#include "../../../core/util/OptionSet.h"
#include "../../../core/util/ClosedIntRange.h"

class QByteArray;
class QString;

class PsiBlastWrapper;

class BlastTask : public IEntityBuilderTask
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    BlastTask(int querySeqId,
              const BioString &bioString,
              const ClosedIntRange &queryRange,
              const OptionSet &psiBlastOptionSet,
              const QDir &outDirectory,
              const QString &name,
              const BlastDatabaseSpec &databaseSpec);

    QDir outDirectory() const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void start();
    void kill();


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    IEntity *createFinalEntity(const QString &entityName) const;


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onProgressChanged(int currentIteration, int totalIterations, int currentStep, int totalSteps);
    void onError(int id, const QString &errorMessage);
    void onFinished(int id, const QByteArray &output);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool correctBlastXmlOrdIds(const QByteArray &blastXml, QIODevice *outputDevice);


    // ------------------------------------------------------------------------------------------------
    // Private members
    int querySeqId_;
    BioString bioString_;
    ClosedIntRange queryRange_;
    OptionSet psiBlastOptionSet_;
    QDir outDirectory_;                 //!< Directory where to store the BLAST output file
    PsiBlastWrapper *psiBlastWrapper_;
    BlastDatabaseSpec databaseSpec_;

    QString outFile_;                   //!< Blast file is stored here!
};

#endif // BLASTTASK_H
