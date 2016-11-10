/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>

#include "BlastSequenceFetcher.h"
#include "../macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
BlastSequenceFetcher::BlastSequenceFetcher(QObject *parent)
    : BlastDbCmdBase(parent),
      protoGrammar_(eUnknownGrammar),
      fatalRuntimeError_(false)
{
    // To combine error reporting of unfound ids with found ids
    process_->setProcessChannelMode(QProcess::MergedChannels);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns BioStringVector
  */
BioStringVector BlastSequenceFetcher::bioStrings() const
{
    return bioStrings_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * Empty id values are not permitted and will flag an error.
  *
  * @param ids [const QStringList &]
  * @param blastDatabasePath [const QString &]
  * @param grammar [const Grammar]
  */
void BlastSequenceFetcher::fetch(const QStringList &ids, const QString &blastDatabasePath, const Grammar grammar)
{
    preStartCheck();    // Will throw if not properly setup

    ASSERT(ids.size() > 0);
    if (ids.size() == 0)
    {
        emit finished(BioStringVector());
        return;
    }

    if (blastDatabasePath.isEmpty())
    {
        emit error(0, "No BLAST database specified");
        return;
    }

    int invalidId = findInvalidId(ids);
    if (invalidId != -1)
    {
        if (ids.at(invalidId).isEmpty())
            emit error(0, QString("Empty sequence identifier (Number: %1)").arg(invalidId + 1));
        else
            emit error(invalidId, QString("Invalid sequence identifier: %1").arg(ids.at(invalidId)));
        return;
    }

    // Reset
    bioStrings_.clear();
    fatalRuntimeError_ = false;
    runtimeErrorMessage_.clear();

    ids_ = ids;
    protoGrammar_ = grammar;
    process_->start(BlastDbCmdBase::blastDbCmdPath(), QStringList()
                    // Interestingly, this also causes the not found errors to be properly interleaved with ones that
                    // are found. Thus, the target_only argument is quite essential; however, it causes problems if a
                    // blast database has seqid files and user is querying with gnl|BL_ORD_ID|XXX id strings.
                    << "-target_only"               // Only output one line per id
                    << "-db" << blastDatabasePath   // Source database
                    << "-outfmt" << "Success:%s"    // Only output the sequence data and use a Success prefix
                    << "-entry_batch" << "-");      // Wait for input on standard input
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param exitCode [int]
  */
void BlastSequenceFetcher::handleExitCode(int /* exitCode */)
{
    if (!fatalRuntimeError_)
        emit finished(bioStrings_);
    else
        emit error(id(), runtimeErrorMessage_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * This method is also called for any error output because the stdout / stderr channels are merged (see constructor).
  */
void BlastSequenceFetcher::onReadyReadStandardOutput()
{
    process_->setReadChannel(QProcess::StandardOutput);
    while (process_->canReadLine())
    {
        QByteArray line = process_->readLine().trimmed();

        ASSERT(!line.startsWith("USAGE"));
        // This assertion should be true because the extractSearchId does not allow multiple parts. If it does occur,
        // another related error has occurred (e.g. not placing newlines between multiple ids)
        ASSERT(!(line.startsWith("Error: FASTA-style ID") && line.endsWith("has too many parts.")));
        ASSERT(line.isEmpty() == false);

        if (line.startsWith("Success:"))
        {
            bioStrings_ << BioString(line.mid(8), protoGrammar_);
        }
        else if (line.startsWith("Error: Entry not found") ||
                 (line.startsWith("Error:") && line.endsWith("OID not found")))
        {
            bioStrings_ << BioString(protoGrammar_);
        }
        else if (line.startsWith("BLAST query/options error:"))
        {
            // Do nothing :)
        }
        else
        {
            // Example: if (line.startsWith("BLAST Database error"))
            fatalRuntimeError_ = true;
            runtimeErrorMessage_ = line;
            kill(line);
            return;
        }

        emit progressChanged(bioStrings_.size(), ids_.size());
    }
}

/**
  */
void BlastSequenceFetcher::onProcessStarted()
{
    ASSERT(ids_.isEmpty() == false);
    ASSERT(isRunning());

    foreach (const QString &id, ids_)
    {
        process_->write(extractSearchId(id).toAscii());
        process_->write("\n");
    }

    // Once the write channel (ie. stdin) is closed, blastdbcmd begins the search and retrieve process
    process_->closeWriteChannel();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * An invalid id is one that cannot have a search id extracted.
  *
  * @param ids [const QStringList &]
  * @returns int
  */
int BlastSequenceFetcher::findInvalidId(const QStringList &ids) const
{
    int i = 0;
    foreach (const QString &id, ids)
    {
        if (extractSearchId(id).isEmpty())
            return i;

        ++i;
    }

    return -1;
}

/**
  * Except for gi, only the first three pipe-separated values are considered. For instance, sp|Q03423|CHEY_ECOLI|abc
  * only the part sp|Q03423|CHEY_ECOLI will be used, the abc portion will be silently ignored.
  *
  * @param id [const QString &]
  * @returns QString
  */
QString BlastSequenceFetcher::extractSearchId(const QString &id) const
{
    if (id.isEmpty())
        return QString();

    QStringList parts = id.split("|");
    if (parts.size() < 2 || parts.first().trimmed().isEmpty())
        return QString("lcl|%1").arg(id);

    // The only ones allowed to have an empty second part is pir and prf
    if (parts.first() != "pir" && parts.at(1) != "prf")
    {
        // Check that the gi is validly formatted
        if (parts.first() == "gi")
        {
            // Check that the second part is not empty and it consists solely of digits
            if (parts.at(1).isEmpty())
                return QString();

            const QChar *x = parts.at(1).constBegin();
            for (; !x->isNull(); ++x)
                if (x->isDigit() == false)
                    return QString();

            return QString("gi|%1").arg(parts.at(1));
        }
        else if (parts.first() == "gnl")
        {
            if (parts.size() != 3 || parts.at(1) != "BL_ORD_ID")
                return QString();

            // Check that the third part is all digits
            const QChar *x = parts.at(2).constBegin();
            for (; !x->isNull(); ++x)
                if (x->isDigit() == false)
                    return QString();

            return id;
        }

        // Otherwise, assume everything is good
        if (parts.size() == 2)
            return parts.first() + "|" + parts.at(1);
        else
            return parts.first() + "|" + parts.at(1) + "|" + parts.at(2);
    }
    else    // Dealing with pir or prf
    {
        if (parts.size() < 3 ||                 // Requires three parts
            parts.at(1).isEmpty() == false ||   // The middle part must be empty
            parts.at(2).isEmpty())              // The last part must not be empty
        {
            return QString();
        }

        return parts.at(0) + "||" + parts.at(2);
    }
}

