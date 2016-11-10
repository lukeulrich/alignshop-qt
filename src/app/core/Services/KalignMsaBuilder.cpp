/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>

#include "KalignMsaBuilder.h"
#include "../Parsers/FastaParser.h"
#include "../constants/KalignConstants.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
KalignMsaBuilder::KalignMsaBuilder(QObject *parent)
    : AbstractMsaBuilder(parent),
      progress_(0)
{
    setOptionProfile(constants::Kalign::kKalignOptionProfile);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString KalignMsaBuilder::friendlyProgramName() const
{
    return constants::Kalign::kFriendlyKalignName;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param sequences [const QVector<IdBioString> &]
  */
void KalignMsaBuilder::align(const QVector<IdBioString> &sequences)
{
    if (sequences.size() < 2)
    {
        emit error(id(), "At least 2 sequences are required to compute an alignment");
        return;
    }

    reset();

    sequences_ = sequences;

    // Create output file for saving data
    QTemporaryFile tempOutFile(QDir::tempPath() + QDir::separator() + "kalign.out.XXXXXX");
    if (!tempOutFile.open())
    {
        emit error(id(), "Unable to create temporary output file");
        return;
    }
    tempOutFile.close();
    tempOutFile.setAutoRemove(false);
    outFile_ = tempOutFile.fileName();
    options_.set(constants::Kalign::kOutFileOpt, outFile_);

    progress_ = 0;

    execute();
}

/**
  * Rather than create a separate file containing the sequence and pass this information to Kalign as a command line
  * argument, we provide the query sequence via STDIN. This method submits this information to Kalign in the  FASTA
  * format and then closes the stdin stream. Otherwise, the process would simply hang waiting for user input.
  */
void KalignMsaBuilder::onProcessStarted()
{
    ASSERT(sequences_.isEmpty() == false);
    ASSERT(isRunning());

    // Now that the process has successfully begun, send the sequence to the process by writing to its standard input.
    // TODO: Check that all data was actually written!
    foreach (const IdBioString &sequence, sequences_)
    {
        // TODO: Check that we actually wrote this data to the disk
        process_->write(">");
        process_->write(QByteArray::number(sequence.id_));
        process_->write("\n");
        process_->write(sequence.bioString_.asByteArray());
        process_->write("\n");
    }

    // Once we close the write channel (ie. stdin), kalign will initiate the alignment process
    process_->closeWriteChannel();
}

/**
  */
void KalignMsaBuilder::onReadyReadStandardError()
{
    static int lastPercentDone = 0;

    QByteArray data = process_->readAllStandardError();
    int offset = 0;
    int y = data.indexOf(" percent done");
    while (y != -1 && y > 0)
    {
        // Compute the next offset for y
        offset = y + 13; // 13 = length(" percent done")

        --y;
        int x = y;
        while (x >= 0 && isdigit(data.at(x)))
            --x;

        bool ok = false;
        int percentDone = data.mid(x, y-x+1).toInt(&ok);
        if (!ok)
            continue;

        if (percentDone != lastPercentDone)
        {
            if (percentDone < lastPercentDone)
                lastPercentDone = 0;
            progress_ += (percentDone - lastPercentDone);
            emit progressChanged(progress_, 200);           // 200 because it goes through (2) 0 - 100% ranges.
        }

        y = data.indexOf(" percent done", offset);
        lastPercentDone = percentDone;
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void KalignMsaBuilder::handleFinished()
{
    try
    {
        // Parse out the alignment data
        ASSERT(outFile_.isEmpty() == false);

        // To toggle that we are completely finished
        emit progressChanged(200, 200);

        emit alignFinished(parseAlignment(outFile_));
        emit finished(id(), QByteArray());

        // Remove the temporary file
        QFile::remove(outFile_);
        outFile_.clear();
    }
    catch(QString &errorMessage)
    {
        emit error(id(), errorMessage);
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param file [const QString &]
  * @returns QVector<IdBioString>
  */
QVector<IdBioString> KalignMsaBuilder::parseAlignment(const QString &file)
{
    FastaParser parser;

    SequenceParseResultPod resultPod = parser.parseFile(file);
    QVector<SimpleSeqPod> simpleSeqs = resultPod.simpleSeqPods_;

    if (sequences_.size() != simpleSeqs.size())
        throw QString("Alignment does not contain equal number of input sequences");

    Grammar grammar = sequences_.first().bioString_.grammar();

    QVector<IdBioString> alignedSequences;
    alignedSequences.reserve(simpleSeqs.size());
    foreach (const SimpleSeqPod &simpleSeq, simpleSeqs)
        alignedSequences << IdBioString(simpleSeq.name_.toInt(), BioString(simpleSeq.sequence().asByteArray(), grammar));

    return alignedSequences;
}
