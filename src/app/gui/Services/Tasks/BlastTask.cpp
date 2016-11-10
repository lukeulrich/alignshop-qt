/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QByteArray>
#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QTemporaryFile>

#include <QtXml/QXmlStreamReader>
#include <QtXml/QXmlStreamWriter>

#include "BlastTask.h"

#include "../../../core/Entities/BlastReport.h"
#include "../../../core/Services/PsiBlastWrapper.h"
#include "../../../core/constants/PsiBlastConstants.h"
#include "../../../core/constants.h"
#include "../../../core/global.h"
#include "../../../core/macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param querySeqId [int]
  * @param bioString [const BioString &]
  * @param queryRange [const ClosedIntRange &]
  * @param psiBlastOptions [const OptionSet &]
  * @param outDirectory [const QDir &]
  * @param name [const QString &]
  * @param databaseSpec [const BlastDatabaseSpec &]
  */
BlastTask::BlastTask(int querySeqId,
                     const BioString &bioString,
                     const ClosedIntRange &queryRange,
                     const OptionSet &psiBlastOptionSet,
                     const QDir &outDirectory,
                     const QString &name,
                     const BlastDatabaseSpec &databaseSpec)
    : IEntityBuilderTask(Ag::Leaf, name),
      querySeqId_(querySeqId),
      bioString_(bioString),
      queryRange_(queryRange),
      psiBlastOptionSet_(psiBlastOptionSet),
      outDirectory_(outDirectory.absolutePath()),
      psiBlastWrapper_(nullptr),
      databaseSpec_(databaseSpec)
{
    // Necessary to ensure parsing output works as expected (XmlOutput)
    ASSERT(psiBlastOptionSet_.contains("-outfmt", "5"));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QDir
  */
QDir BlastTask::outDirectory() const
{
    return outDirectory_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void BlastTask::start()
{
    // Lazy initialization
    if (psiBlastWrapper_ == nullptr)
    {
        QString psiBlastPath = QCoreApplication::applicationDirPath() +
                               QDir::separator() +
                               constants::kPsiBlastRelativePath;

        psiBlastWrapper_ = new PsiBlastWrapper(this);
        psiBlastWrapper_->setProgram(psiBlastPath);
        psiBlastOptionSet_.set(constants::PsiBlast::kThreadsOpt, nThreads());
        if (!psiBlastWrapper_->setOptions(psiBlastOptionSet_))
        {
            onError(querySeqId_, "Error setting PSI-BLAST options");
            return;
        }

        connect(psiBlastWrapper_, SIGNAL(progressChanged(int,int,int,int)), SLOT(onProgressChanged(int,int,int,int)));
        connect(psiBlastWrapper_, SIGNAL(error(int,QString)), SLOT(onError(int,QString)));
        connect(psiBlastWrapper_, SIGNAL(finished(int,QByteArray)), SLOT(onFinished(int,QByteArray)));
    }

    setStatus(Ag::Running);
    try
    {
        psiBlastWrapper_->psiblast(querySeqId_, bioString_);
    }
    catch (QString &error)
    {
        onError(querySeqId_, error);
    }
}

/**
  */
void BlastTask::kill()
{
    if (psiBlastWrapper_ == nullptr)
        return;

    // Currently, the psiblast kill operation is synchronous so it is safe not to check for a signal
    psiBlastWrapper_->kill();

    if (shouldDeleteWhenFinished())
        deleteLater();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @returns IEntity *
  */
IEntity *BlastTask::createFinalEntity(const QString &entityName) const
{
    if (status() != Ag::Finished)
        return nullptr;

    ASSERT(outFile_.isEmpty() == false);
    ASSERT(QFile::exists(outFile_));

    return BlastReport::createEntity(querySeqId_,
                                     queryRange_,
                                     bioString_,
                                     QFileInfo(outFile_).fileName(),
                                     entityName,
                                     databaseSpec_);
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
void BlastTask::onProgressChanged(int currentIteration, int totalIterations, int currentStep, int totalSteps)
{
    setProgress( static_cast<double>((currentIteration - 1) * totalSteps + currentStep) /
                 static_cast<double>(totalIterations * totalSteps));
}

/**
  * @param id [int]
  * @param errorMessage [const QString &]
  */
void BlastTask::onError(int /* id */, const QString &errorMessage)
{
    setStatus(Ag::Error);
    setNote(errorMessage);

    emit error(this);
}

/**
  * @param id [int]
  * @param output [const QByteArray &]
  */
void BlastTask::onFinished(int id, const QByteArray &output)
{
    // Save the output to a file
    if (!outDirectory_.mkpath("."))
    {
        onError(id, QString("Unable to create BLAST output directory: %1").arg(outDirectory_.path()));
        return;
    }

    QTemporaryFile blastFile(outDirectory_.path() + QDir::separator() + QString("%1.XXXXXX.blast").arg(querySeqId_));
    if (!blastFile.open())
    {
        onError(id, QString("Unable to create BLAST output file: %1").arg(blastFile.fileName()));
        return;
    }

    // See notes on 8 Nov 2011 for more details.
    // Briefly, the Blast XML output (at least with BLAST+ version 2.2.25), display BL_ORD_ID values *per volume*. Thus,
    // with multi volume databases, the results will not import successfully; however, the pseudo accession reported as
    // the id value works does correspond properly to the global oid. The correctBlastXmlOrdIds method parses the XML
    // output and replaces all gnl|BL_ORD_ID|{wrong oid} with gnl|BL_ORD_ID|{accession} if applicable.
    //
    // Again this only applies to sequence databases formatted without the parse_seqids option.
    if (!correctBlastXmlOrdIds(output, &blastFile))
    {
        onError(id, QString("Error writing to file (%1): %2").arg(blastFile.fileName()).arg(blastFile.errorString()));
        return;
    }

    blastFile.close();
    blastFile.setAutoRemove(false);

    // Qt automatically makes temporary files with read/write for user only; since we are co-opting its purpose to make
    // a normal file, restore "default" permissions.
    // ISSUE: This could obviously be a problem with a non 644 umask for files
    blastFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
                             QFile::ReadUser | QFile::WriteUser |
                             QFile::ReadGroup |
                             QFile::ReadOther);

    outFile_ = blastFile.fileName();

    setStatus(Ag::Finished);

    emit done(this);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param blastXml [const QByteArray &]
  * @param outputDevice [QIODevice *]
  * @returns bool
  */
bool BlastTask::correctBlastXmlOrdIds(const QByteArray &blastXml, QIODevice *outputDevice)
{
    QXmlStreamReader reader(blastXml);
    QXmlStreamWriter writer(outputDevice);

    QByteArray buffer;
    QBuffer bufferStream(&buffer);
    bufferStream.open(QIODevice::WriteOnly);

    bool foundOrdId = false;

    while (!reader.atEnd())
    {
        switch (reader.readNext())
        {
        case QXmlStreamReader::StartDocument:
            writer.setCodec(reader.documentEncoding().toString().toAscii());
            writer.writeStartDocument(reader.documentVersion().toString(), reader.isStandaloneDocument());
            break;
        case QXmlStreamReader::EndDocument:
            writer.writeEndDocument();
            break;
        case QXmlStreamReader::StartElement:
            if (reader.name() == "Hit_id")
            {
                QString text = reader.readElementText();
                if (reader.hasError())
                    return false;

                ASSERT(text.isEmpty() == false);
                if (!text.startsWith("gnl|BL_ORD_ID|"))
                {
                    writer.writeStartElement(reader.name().toString());
                    writer.writeAttributes(reader.attributes());
                    writer.writeCharacters(text);
                    writer.writeEndElement();
                    break;
                }
                else
                {
                    foundOrdId = true;
                    writer.setDevice(&bufferStream);
                    break;
                }
            }
            else if (foundOrdId)
            {
                if (reader.name() == "Hit_accession")
                {
                    QString accession = reader.readElementText();
                    if (reader.hasError())
                        return false;

                    // Now update the bl_ord_id and switch writer back to the default device
                    outputDevice->write("<Hit_id>gnl|BL_ORD_ID|");
                    outputDevice->write(accession.toAscii());
                    outputDevice->write("</Hit_id>");
                    outputDevice->write(buffer);

                    writer.setDevice(outputDevice);
                    writer.writeStartElement(reader.name().toString());
                    writer.writeAttributes(reader.attributes());
                    writer.writeCharacters(accession);
                    writer.writeEndElement();

                    buffer.clear();
                    bufferStream.reset();
                    foundOrdId = false;
                    break;
                }
            }

            writer.writeStartElement(reader.name().toString());
            writer.writeAttributes(reader.attributes());
            break;
        case QXmlStreamReader::EndElement:
            writer.writeEndElement();
            break;
        case QXmlStreamReader::Characters:
            writer.writeCharacters(reader.text().toString());
            break;
        case QXmlStreamReader::Comment:
            writer.writeComment(reader.text().toString());
            break;

        default:
            break;
        }
    }

    return true;
}
