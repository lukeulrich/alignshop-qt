/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTSEQUENCEFETCHER_H
#define BLASTSEQUENCEFETCHER_H

#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "BlastDbCmdBase.h"
#include "../BioString.h"
#include "../global.h"

/**
  * BlastSequenceFetcher conveniently wraps fetching sequences from a BLAST database using the NCBI blastdbcmd tool.
  *
  * The blastdbcmd tool returns the requested data per id per line. Many times, multiple ID's are associated with a
  * single sequence. Normally, blastdbcmd maps a given id to all its associated IDs, and returns the requested data for
  * all IDs. To avoid this, the -target_only option is used.
  *
  * If data is not found for a given ID, blastdbcmd writes a single-line error message to stderr and continues.
  * All other results are written to stdout. To preserve the relationship between the list of requested ids and the
  * resultant result vector, an empty BioString denotes an id that was not found.
  *
  * Currently, only fetches the full-length sequence data for each id. Moreover, all ids must be properly referenced in
  * relation to their source. For example, GenBank identifiers, should be prefixed with "gi|". Isolated ids without any
  * textual source will cause an error to be emitted. If the ids are custom, then the "lcl|{id}" format should be used.
  * In the event the BLAST database has been sequenced without seqids, then the "gnl|BL_ORD_ID|{oid}" form should be
  * requested.
  *
  * Turns out that if an id is not found with blastdbcmd it will output a non-zero exit code. AbstractProcessWrapper
  * would by default see this is a problem and emit the error signal; however, since this class does not consider that
  * an error condition, the handleExitCode virtual method is overriden to either emit an error (indicating a runtime
  * that was captured during output processing) or that it finished successfully.
  */
class BlastSequenceFetcher : public BlastDbCmdBase
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    explicit BlastSequenceFetcher(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioStringVector bioStrings() const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void fetch(const QStringList &ids, const QString &blastDatabasePath, const Grammar grammar = eUnknownGrammar);


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void progressChanged(int currentStep, int totalSteps);
    void finished(const BioStringVector &bioStrings);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void handleExitCode(int exitCode);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onReadyReadStandardOutput();                   //!< Called when there is data available on standard error (e.g. progress messages)
    void onProcessStarted();                            //!< Called when the process has started


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    int findInvalidId(const QStringList &ids) const;    //!< Checks each id for a valid format; returns the index of the first invalid id found or -1 if none are found
    QString extractSearchId(const QString &id) const;   //!< Extracts from id the proper search string to use with blastdbcmd


    // ------------------------------------------------------------------------------------------------
    // Private members
    QStringList ids_;
    Grammar protoGrammar_;
    BioStringVector bioStrings_;

    bool fatalRuntimeError_;
    QString runtimeErrorMessage_;
};


#endif // BLASTSEQUENCEFETCHER_H
