/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSet>
#include <QtCore/QTextStream>

#include "BlastDatabaseFinder.h"
#include "../constants.h"
#include "../macros.h"

// %f means the BLAST database absolute file name path
// %p means the BLAST database molecule type [Protein or Nucleotide]
// %t means the BLAST database title
// %d means the date of last update of the BLAST database [IGNORE]
// %l means the number of bases/residues in the BLAST database
// %n means the number of sequences in the BLAST database
// %U means the number of bytes used by the BLAST database
//
// Because the database title and file name path may contain somewhat arbitrary characters, we delimit
// these fields with special characters to make parsing easier and more accurate.
// {file name path}** {molecule type} {letters} {sequences} {bytes} {database title}
static const QString kListOutFmt = "%f** %p %l %n %U %t";

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
BlastDatabaseFinder::BlastDatabaseFinder(QObject *parent)
    : BlastDbCmdBase(parent)
{
    connect(this, SIGNAL(finished(int,QByteArray)), SLOT(onFinished(int,QByteArray)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * Throws an error if the blastdbcmd does not point to a valid exectuable file.
  *
  * @param id [int]
  * @param path [const QString &]
  */
void BlastDatabaseFinder::findBlastDatabases(int id, const QString &path)
{
    preStartCheck();    // Will throw if not properly setup

    id_ = id;
    findPath_ = path;

    process_->start(BlastDbCmdBase::blastDbCmdPath(), QStringList()
                    << "-list" << findPath_ << "-list_outfmt" << kListOutFmt);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param id [int]
  * @param output [const QByteArray &]
  */
void BlastDatabaseFinder::onFinished(int id, const QByteArray &output)
{
    emit foundBlastDatabases(id, findPath_, parseFindOutput(output));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * A volume is determined from its file extension and this method returns true if the last three characters are .\d\d.
  * Like other situations, the file is not necessarily a traditional file, but rather the name of the database reported
  * by the blastdbcmd.
  *
  * Since multiple blast databases may be combined into a virtual single blast database, it is not necessarily possible
  * to distinguish which blast databases are simply volumes or an intended combination. Therefore, only those volumes
  * with an explicit .\d\d suffix are deemed blast volumes by this method.
  *
  * @param pod [const BlastDatabaseMetaPod &]
  * @returns bool
  */
bool BlastDatabaseFinder::isBlastVolume(const BlastDatabaseMetaPod &pod) const
{
    return isBlastVolumeString(pod.file_);
}

bool BlastDatabaseFinder::isBlastVolumeString(const QString &string) const
{
    // The filename must have at least four characters (3 for the volume file extension)
    if (string.length() < 4)
        return false;

    // Get the last three characters
    const QChar *x = string.constData() + string.length() - 3;
    return *x == '.' &&
           (x+1)->isDigit() &&
           (x+2)->isDigit();
}

/**
  * Blast database volume files are identified as those files with the same name and a .\d\d extension. This method
  * simply filters out all those that have a file with a .\d\d suffix.
  *
  * @param output [const QByteArray &]
  * @returns QVector<BlastDatabaseMetaPod>
  */
QVector<BlastDatabaseMetaPod> BlastDatabaseFinder::parseFindOutput(const QByteArray &output) const
{
    if (output.isEmpty())
        return QVector<BlastDatabaseMetaPod>();

    QSet<QString> podFilesMissingDependency;
    QVector<BlastDatabaseMetaPod> pods;
    QList<QByteArray> lines = output.trimmed().split('\n');
    foreach (const QByteArray &line, lines)
    {
        BlastDatabaseMetaPod pod;
        if (parseLine(line, pod) && !isBlastVolume(pod))
        {
            if (pod.missingDependency_)
            {
                if (podFilesMissingDependency.contains(pod.file_))
                    continue;
                podFilesMissingDependency << pod.file_;
            }
            pods << pod;
        }
    }

    return pods;
}

/**
  * QDir::canonicalPath() and QFileInfo::canonicalPath() both return an empty string if the corresponding file does not
  * exist. There are two cases where it is possible to have a valid blast database but the "file" does not exist:
  * 1) BLAST database was created from a fasta file, which was subsequently removed
  * 2) Alias files
  *
  * Thus, it is important not to consider pod.file_ as actually referencing a true file and only use the canonical path
  * methods on its parent directory.
  *
  * @param line [const QByteArray &]
  * @param pod [BlastDatabaseMetaPod &]
  * @returns bool
  */
bool BlastDatabaseFinder::parseLine(const QByteArray &line, BlastDatabaseMetaPod &pod) const
{
    if (line.isEmpty() || line.length() < 16)
        return false;

    // 1) Find the file path
    int doubleStarPos = line.indexOf("**");
    if (doubleStarPos < 1)  // If at 0 or -1, then it is invalid
        return parseLineAsMissingDependency(line, pod);

    pod.file_ = line.mid(0, doubleStarPos);

    QFileInfo fileInfo(pod.file_);
    QString parentDirectory = fileInfo.path();
    pod.file_ = QDir::cleanPath(QDir(parentDirectory).canonicalPath()) + "/" + fileInfo.fileName();

    // Setup the parse stream
    QTextStream textStream(line);
    if (!textStream.seek(doubleStarPos + 3))
        return false;

    bool ok = false;
    QByteArray buffer;

    // 2) Molecule type
    textStream >> buffer;
    ASSERT(buffer == "Protein" || buffer == "Nucleotide");
    pod.isProtein_ = (buffer == "Protein") ? eTrue : eFalse;

    // 3) Number of letters
    textStream >> buffer;
    pod.nLetters_ = buffer.toULongLong(&ok);
    if (!ok)
        return false;

    // 4) Number of sequences
    textStream >> buffer;
    pod.nSequences_ = buffer.toUInt(&ok);
    if (!ok)
        return false;

    // 5) Number of bytes
    textStream >> buffer;
    pod.nBytes_ = buffer.toULongLong(&ok);
    if (!ok)
        return false;

    // 6) BLAST database title
    pod.title_ = textStream.readAll().trimmed();

    return true;
}

bool BlastDatabaseFinder::parseLineAsMissingDependency(const QByteArray &line, BlastDatabaseMetaPod &pod) const
{
    static QRegExp lineRegexp("^Could not find volume or alias file \\(([^)]+)\\) referenced in alias file \\(([^)]+)\\)");
    int pos = lineRegexp.exactMatch(line);
    if (pos == -1)
        return false;

    QString missingAliasFile = lineRegexp.cap(1);
    QString sourceFile = lineRegexp.cap(2);

    pod.file_ = sourceFile;
    if (isBlastVolumeString(sourceFile))
        pod.file_.chop(3);      // To remove the .\d\d extension, which is guaranteed by the isBlastVolume test
    QFileInfo fileInfo(pod.file_);
    QString parentDirectory = fileInfo.path();
    pod.file_ = QDir::cleanPath(QDir(parentDirectory).canonicalPath()) + "/" + fileInfo.fileName();

    QString missingBlastDatabase = missingAliasFile;
    if (isBlastVolumeString(missingBlastDatabase))
        missingBlastDatabase.chop(3);
    pod.missingDependency_ = true;
    pod.note_ = QString("Could not find volume or alias file, %1. Download or place the %2 BLAST database in this directory.").arg(missingAliasFile).arg(missingBlastDatabase);
    return true;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Deprecated
/**
  * To determine if a blast database has been parsed with seqids, it must have the following files:
  * <database>.[pn]og
  * <database>.[pn]sd
  * <database>.[pn]si
  *
  * and they must have a non-zero filesize.
  *
  * @param blastDatabaseMetaPods [const QVector<BlastDatabaseMetaPod> &]
  * @returns QVector<BlastDatabaseMetaPod>
  */
QVector<BlastDatabaseMetaPod> BlastDatabaseFinder::findPodsWithSeqIds(const QVector<BlastDatabaseMetaPod> &pods)
{
    QVector<BlastDatabaseMetaPod> podsWithSeqIds;

    foreach (const BlastDatabaseMetaPod &pod, pods)
    {
        QFileInfo og(pod.file_ + "." + ((pod.isProtein()) ? "p" : "n") + "og");
        QFileInfo sd(pod.file_ + "." + ((pod.isProtein()) ? "p" : "n") + "sd");
        QFileInfo si(pod.file_ + "." + ((pod.isProtein()) ? "p" : "n") + "si");

        if (og.exists() && og.size() > 0 &&
            sd.exists() && sd.size() > 0 &&
            si.exists() && si.size() > 0)
        {
            podsWithSeqIds << pod;
            continue;
        }

        // It is possible that this is a multi-volume database and thus must check for the presence of these files in at
        // least one of the volumes
        og = QFileInfo(pod.file_ + ".00." + ((pod.isProtein()) ? "p" : "n") + "og");
        sd = QFileInfo(pod.file_ + ".00." + ((pod.isProtein()) ? "p" : "n") + "sd");
        si = QFileInfo(pod.file_ + ".00." + ((pod.isProtein()) ? "p" : "n") + "si");
        if (og.exists() && og.size() > 0 &&
            sd.exists() && sd.size() > 0 &&
            si.exists() && si.size() > 0)
        {
            podsWithSeqIds << pod;
            continue;
        }
    }

    return podsWithSeqIds;
}

