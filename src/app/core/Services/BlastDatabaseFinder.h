/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTDATABASEFINDER_H
#define BLASTDATABASEFINDER_H

#include <QtCore/QVector>

#include "BlastDbCmdBase.h"
#include "../PODs/BlastDatabaseMetaPod.h"
#include "../global.h"

/**
  * Blast database configuration and discovery
  * Sequence retrieval is done with another tool
  *
  * What happens if the path does not exist?
  * Do multiple paths at once?
  *
  * BlastDbCmd can only search one path at a time.
  * If the path does not exist, it does not throw an exception
  *
  * Another advantage of requesting the BLAST database stats is that blastdbcmd will actually check pal files for
  * properly referenced databases and output an error message if they are not found.
  *
  * ISSUE:
  * o It is possible to have a directory with both valid and invalid BLAST databases. In these cases, blastdbcmd exits
  *   with an error exit code despite returning valid results for any it did find. There needs to be a way to
  *   distinguish these cases.
  */
class BlastDatabaseFinder : public BlastDbCmdBase
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit BlastDatabaseFinder(QObject *parent = nullptr);


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void findBlastDatabases(int id, const QString &path);       //!< Searches for blast databases in path (not recursive) that are formatted with the parse_seqids option


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    //! Emitted when the findBlastDatabases method has finished; each BlastDatabaseMetaPod will be initialized and its file_ set to its absolute path
    void foundBlastDatabases(int id, const QString &path, const BlastDatabaseMetaPodVector &blastDatabaseMetaPods);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onFinished(int id, const QByteArray &output);          //!< Emitted when the blastdbcmd has finished and its results are stored in output


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool isBlastVolume(const BlastDatabaseMetaPod &pod) const;  //!< Returns true if the BLAST database pointed to by pod is a volume; false otherwise
    bool isBlastVolumeString(const QString &string) const;      //!< Returns true if string represents a BLAST volume; false otherwise
    //! Parses output and returns a vector of core BlastDatabaseMetaPods (excluding volumes)
    QVector<BlastDatabaseMetaPod> parseFindOutput(const QByteArray &output) const;
    //!< Parses line for blast database information and stores this information in pod; returns true if parse was successful, false otherwise
    bool parseLine(const QByteArray &line, BlastDatabaseMetaPod &pod) const;
    bool parseLineAsMissingDependency(const QByteArray &line, BlastDatabaseMetaPod &pod) const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    QString findPath_;                  //!< Current path being searched


    // ------------------------------------------------------------------------------------------------
    // Deprecated
    //! Finds and returns those BlastDatabaseMetaPods that are formatted with the parse_seqids option
    QVector<BlastDatabaseMetaPod> findPodsWithSeqIds(const QVector<BlastDatabaseMetaPod> &pods);
};

#endif // BLASTDATABASEFINDER_H
