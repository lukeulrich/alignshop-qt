/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>

#include <cstdio>              // For sscanf and sscanf_s

#include "ClustalWMsaBuilder.h"
#include "../Parsers/FastaParser.h"
#include "../constants/ClustalWConstants.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
ClustalWMsaBuilder::ClustalWMsaBuilder(QObject *parent)
    : AbstractMsaBuilder(parent),
      totalPairwise_(0)
{
    setOptionProfile(constants::ClustalW::kClustalWOptionProfile);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString ClustalWMsaBuilder::friendlyProgramName() const
{
    return "ClustalW";
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param sequences [const QVector<IdBioString> &]
  */
void ClustalWMsaBuilder::align(const QVector<IdBioString> &sequences)
{
    if (sequences.size() < 2)
    {
        emit error(id(), "At least 2 sequences are required to compute an alignment");
        return;
    }

    reset();

    sequences_ = sequences;

    // Create input file
    QTemporaryFile tempInFile(QDir::tempPath() + QDir::separator() + "clustalw.in.XXXXXX.faa");
    if (!tempInFile.open())
    {
        emit error(id(), "Unable to create temporary file");
        return;
    }

    foreach (const IdBioString &sequence, sequences_)
    {
        // TODO: Check that we actually wrote this data to the disk
        tempInFile.write(">");
        tempInFile.write(QByteArray::number(sequence.id_));
        tempInFile.write("\n");
        tempInFile.write(sequence.bioString_.asByteArray());
        tempInFile.write("\n");
    }
    tempInFile.close();

    // Create output file for saving data
    QTemporaryFile tempOutFile(QDir::tempPath() + QDir::separator() + "clustalw.out.XXXXXX");
    if (!tempOutFile.open())
    {
        emit error(id(), "Unable to create temporary output file");
        return;
    }
    tempOutFile.close();
    tempOutFile.setAutoRemove(false);
    outFile_ = tempOutFile.fileName();
    options_.set(constants::ClustalW::kOutFileOpt, outFile_);

    tempInFile.setAutoRemove(false);
    inFile_ = tempInFile.fileName();
    options_.set(constants::ClustalW::kInFileOpt, inFile_);

    // Output in fasta format
    options_.set(constants::ClustalW::kFormatOpt, constants::ClustalW::kFormatFasta);

    totalPairwise_ = halfSquareTotal(sequences_.size());

    execute(QProcess::ReadOnly);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void ClustalWMsaBuilder::handleFinished()
{
    try
    {
        // Parse out the alignment data
        ASSERT(outFile_.isEmpty() == false);

        QVector<IdBioString> alignment = parseAlignment(outFile_);

        // To toggle that we are completely finished
        emit progressChanged(totalPairwise_, totalPairwise_);
        emit alignFinished(alignment);
        emit finished(id(), QByteArray());
    }
    catch(QString &errorMessage)
    {
        emit error(id(), errorMessage);
    }
    catch(...)
    {
        emit error(id(), "Unable to parse resulting alignment");
    }

    // Remove the temporary files
    QFile::remove(outFile_);
    QFile::remove(inFile_);
    // And remove the guide tree file that ClustalW inevitably creates
    QFile::remove(inFile_.mid(0, inFile_.length() - 4) + ".dnd");

    outFile_.clear();
    inFile_.clear();
}

/**
  * @param first [int]
  * @param second [int]
  * @returns int
  */
int ClustalWMsaBuilder::currentStep(int first, int second) const
{
    ASSERT(first > 0);
    ASSERT(second > first);
    return totalPairwise_ - halfSquareTotal(sequences_.size() - first + 1) + (second - first);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected slots
/**
  */
void ClustalWMsaBuilder::onReadyReadStandardOutput()
{
    QPair<int, int> lastUpdate(-1, -1);

    process_->setReadChannel(QProcess::StandardOutput);
    while (process_->canReadLine())
    {
        QByteArray line = process_->readLine();
        if (!line.startsWith("Sequences ("))
            continue;

        const char *x = line.constData() + 11; // 11 = length("Sequences (")
        int first = -1;
        int second = -1;
        int nFound = sscanf(x, "%d:%d)", &first, &second);
        if (nFound == 2)
        {
            lastUpdate.first = first;
            lastUpdate.second = second;
        }
    }

    if (lastUpdate.first != -1)
        emit progressChanged(currentStep(lastUpdate.first, lastUpdate.second), totalPairwise_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param file [const QString &]
  * @returns QVector<IdBioString>
  */
QVector<IdBioString> ClustalWMsaBuilder::parseAlignment(const QString &file)
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

/**
  * @param nSequences [int]
  * @returns int
  */
int ClustalWMsaBuilder::halfSquareTotal(int rows) const
{
    return ((rows * rows) - rows) / 2.;
}
