/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "MakeBlastDatabaseWrapper.h"
#include "../constants/MakeBlastDbConstants.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
MakeBlastDatabaseWrapper::MakeBlastDatabaseWrapper(QObject *parent)
    : AbstractProcessWrapper(parent),
      fastaTick_(0),
      emittedError_(false)
{
    setOptionProfile(constants::MakeBlastDb::kMakeBlastDbOptionProfile);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int MakeBlastDatabaseWrapper::fastaTick() const
{
    return fastaTick_;
}

/**
  * @returns QString
  */
QString MakeBlastDatabaseWrapper::friendlyProgramName() const
{
    return constants::MakeBlastDb::kFriendlyMakeBlastDbName;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param sequenceFile [const QString &]
  */
void MakeBlastDatabaseWrapper::formatDatabase(const QString &sequenceFile)
{
    if (!QFile::exists(sequenceFile))
    {
        emit error(id(), QString("Sequence file, %1, does not exist.").arg(sequenceFile));
        return;
    }

    QFileInfo fileInfo(sequenceFile);
    if (!fileInfo.isFile() || fileInfo.size() == 0)
    {
        emit error(id(), QString("Sequence fiel, %1, is empty.").arg(sequenceFile));
        return;
    }

    reset();

    sequenceFile_ = sequenceFile;

    options_.set(constants::MakeBlastDb::kInFileOpt, sequenceFile);

    if (fastaTick_ > 0)
        options_.set(constants::MakeBlastDb::kFastaTickOpt, fastaTick_);
    else
        options_.remove(constants::MakeBlastDb::kFastaTickOpt);

    AbstractProcessWrapper::execute(QProcess::ReadOnly);
}

/**
  * @param newFastaTick [int]
  * @returns bool
  */
bool MakeBlastDatabaseWrapper::setFastaTick(int newFastaTick)
{
    ASSERT(newFastaTick >= 0);
    if (newFastaTick >= 0)
    {
        fastaTick_ = newFastaTick;
        return true;
    }

    return false;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void MakeBlastDatabaseWrapper::onReadyReadStandardError()
{
    process_->setReadChannel(QProcess::StandardError);
    while (process_->canReadLine() && !emittedError_)
    {
        QByteArray line = process_->readLine();
        qDebug() << line;
        if (line.startsWith("@progress: ") && line.length() > 11)
        {
            const char *x = line.constData() + 11; // 11 = length("@progress: ")
            int i = 0;
            for (i=0; isdigit(*x); ++i, ++x)
                ;

            if (i)
            {
                bool ok = false;
                int y = line.mid(11, i).toInt(&ok);
                if (ok)
                    emit progressChanged(y);
            }
        }
        else if (line.startsWith("BLAST options error: Input format not supported"))
        {
            emit error(id(), "Error: Unrecognized input file format. Please select another file.");
            emittedError_ = true;
        }
        else if (line.startsWith("BLAST options error: "))
        {
            emit error(id(), "Error: " + line.mid(21));
            emittedError_ = true;
        }
        else if (line.startsWith("Error:"))
        {
            emit error(id(),
                       QString("An error occurred while attempting to read the sequence file. Please verify that "
                               "%1 contains FASTA formatted sequence data.").arg(sequenceFile_));
            emittedError_ = true;
        }
    }
}

/**
  */
void MakeBlastDatabaseWrapper::reset()
{
    emittedError_ = false;
    AbstractProcessWrapper::reset();
}

/**
  */
void MakeBlastDatabaseWrapper::handleError()
{
    if (emittedError_)
        return;

    AbstractProcessWrapper::handleError();
}

/**
  */
void MakeBlastDatabaseWrapper::handleFinished()
{
    emit formatFininshed(sequenceFile_);
    emit finished(id(), QByteArray());
}
