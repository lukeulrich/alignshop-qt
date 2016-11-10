/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "BlastDbCmdBase.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
BlastDbCmdBase::BlastDbCmdBase(QObject *parent)
    : AbstractProcessWrapper(parent),
      id_(0)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString BlastDbCmdBase::blastDbCmdPath() const
{
    return blastDbCmdPath_;
}

/**
  * @returns int
  */
int BlastDbCmdBase::id() const
{
    return id_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * Throws an exception if path does not point to a valid executable program.
  *
  * @param path [const QString &]
  */
void BlastDbCmdBase::setBlastDbCmdPath(const QString &path)
{
    blastDbCmdPath_ = path;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void BlastDbCmdBase::preStartCheck()
{
    // Do not permit running multiple psiblasts at the same time with the same instance
    if (isRunning())
        throw QString("%1 is already running.").arg(blastDbCmdPath_);

    // Check the blastdbcmd path again here in case there have been changes
    checkBlastDbCmdPath(blastDbCmdPath_);

    reset();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param path [const QString &]
  */
void BlastDbCmdBase::checkBlastDbCmdPath(const QString &path) const
{
    if (path.isEmpty())
        throw QString("Please set the path to the blastdbcmd program.");

    // Check that the path to the psiblast executable is valid
    if (!QFile::exists(path))
        throw QString("blastdbcmd program, %1, not found.").arg(path);

    // Check that the psiblast executable is an actual file and runnable
    QFileInfo blastDbCmdPathInfo(path);
    if (!blastDbCmdPathInfo.isFile() || !blastDbCmdPathInfo.isExecutable() || blastDbCmdPathInfo.size() == 0)
        throw QString("blastdbcmd program, %1, is not a valid program.").arg(path);
}
