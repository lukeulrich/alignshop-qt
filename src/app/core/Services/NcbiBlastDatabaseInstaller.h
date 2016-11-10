/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef NCBIBLASTDATABASEINSTALLER_H
#define NCBIBLASTDATABASEINSTALLER_H

#include <QtCore/QHash>
#include <QtCore/QUrl>
#include <QtCore/QVector>

#include <QtNetwork/QUrlInfo>

#include "AbstractBlastDatabaseInstaller.h"
#include "../global.h"

class QFtp;
class QStateMachine;

class GunzipPipe;
class UntarSink;

/**
  * NcbiBlastDatabaseInstaller encapsulates downloading and installing a pre-formatted BLAST database from the NCBI ftp
  * site.
  *
  * All NCBI pre-formatted databases are located at NCBI's ftp site (ftp.ncbi.nih.gov/blast/db) and are stored as
  * gzipped, tarballs (.tar.gz extension). Most of NCBI's BLAST databases are comprised of several files and every BLAST
  * database shares a common prefix. After identifying the exact database to download by defining the prefix and calling
  * execute(), this class will proceed to download, decompress, and unpack all the relevant BLAST database files.
  *
  * This process is broken into the following steps:
  * 1) Setup FTP connection
  * 2) Obtain the list of files in the blast/db directory
  * 3) Filter out those files that do not have the relevant prefix
  * 4) Download, decompress, and unpack all relevant files
  * 5) Emit the relevant signals as appropriate
  *
  * Previously attempted to use a State Machine to perform the above; however, it was far too difficult and later
  * resorted to a simple enum to track the current state.
  *
  * To boost performance, once the file listing is cached for future requests. Call clearListCache() to clear the cache.
  *
  * Also, initially attempted to download files using the QNetworkAccessManager; however, this and FTP servers do not
  * play well at this point. Therefore, I have resorted to using the QFtp class.
  *
  * Originally, canceling the FTP process would send an ABORT signal and wait for it to complete via the onFtpDone slot.
  * However, this can take sometime to complete (https://bugreports.qt.nokia.com/browse/QTBUG-12241), therefore, to make
  * this snappier, simply delete the QFtp instance and reallocate it on the next request.
  *
  * A list of all extracted files is saved. If the user cancels before the install is complete, calling cleanUp will
  * remove these files from the filesystem.
  *
  * All ncbi pre-formatted BLAST databases have the following traits:
  * o All files per database begin with a common prefix (e.g. nr)
  * o Database files compressed tarball with .tar.gz suffix
  * o Have associated md5 hash for the tarball that is suffixed with .md5
  *
  * Possible errors:
  * o No network connection
  * o Ftp could not connect
  * o Invalid blast path
  * o No files with the given prefix found
  * o Miscellaneous ftp error
  *
  * If cancel or other error, must manually send reset before trying again.
  *
  * TODO: Display error messages to the user.
  */
class NcbiBlastDatabaseInstaller : public AbstractBlastDatabaseInstaller
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct an instance with title and parent
    NcbiBlastDatabaseInstaller(const QString &title, QObject *parent = nullptr);
    //! Construct an instance with title, dbPrefix, and parent
    NcbiBlastDatabaseInstaller(const QString &title, const QString &dbPrefix, QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QUrl ncbiBlastDbUrl() const;                            //!< Returns the NCBI url pointing to the blast database ftp directory
    void setNcbiBlastDbUrl(const QUrl &url);                //!< Sets the ncbi blast database url to url


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    virtual void cancel();                                  //!< Cancel any current install
    virtual void cleanUp();                                 //!< Clean up any leftover or intermediate files (e.g. those remaining from an errored process)
    void clearListCache();                                  //!< Clears any cached FTP file listing
    void reset();                                           //!< Resets the installer to a default state and readies for another execution
    bool setDestinationDirectory(const QDir &directory);    //!< Returns true if the destination directory could be set to directory; false otherwise
    bool setPrefix(const QString &dbPrefix);                //!< Returns true if the prefix could be set to dbPrefix; false otherwise


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual void execute();                                 //!< Core method for initiating the install process


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onExtractStarted(const QString &fileName);         //!< Called when the untarSink begins to extract fileName
    void onFtpCommandStarted(int ftpId);                    //!< Called when the the FTP command with ftpId has begun
    void onFtpCommandFinished(int ftpId, bool error);       //!< Called when the the FTP command with ftpId has finished; error is true if an error occurrred, false otherwise
    //! Called when the ftp instance has transferred done out of total data units (typically bytes)
    void onFtpDataTransferProgress(qint64 done, qint64 total);
    void onFtpDone(bool isError);                           //!< Emitted when all pending ftp commands have finished; error is true if an error occurred, false otherwise
    void onFtpListInfo(const QUrlInfo &ftpFile);            //!< Callback for a ftp directory listing
    void onGunzipError(const QString &error);               //!< Callback for handling gunzip errors
    void onUntarError(const QString &error);                //!< Callback for handling untar errors

    void getFtpFileListing();                               //!< Step 1: triggers the ftp commands for fetching a listing of all ftp files in the blast/db directory
    void downloadUnpack();                                  //!< Step 2: downloads and unpacks all relevant database files to the destination directory
    void installSuccess();                                  //!< Step 3: finalization of the database install


private:
    // ------------------------------------------------------------------------------------------------
    // Private structures
    /**
      * RunState is a private struct for tracking the current state of the installer
      */
    enum RunState {
        StartState = 0,                     //!< Initial state
        ErrorState,                         //!< Error state
        GetFileListState,                   //!< Getting ftp files
        DownloadUnpackState                 //!< Downloading and unpacking files
    };

    /**
      * TarBallFile aggregates a ftp tarball and its archived files.
      */
    struct TarBallFile
    {
        QUrlInfo ftpFile_;                                  //!< The source ftp file
        QStringList archivedFiles_;                         //!< List of all files successfully extracted from the archive

        TarBallFile(const QUrlInfo &ftpFile = QUrlInfo())
            : ftpFile_(ftpFile)
        {
        }
    };


    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool acceptFtpFileName(const QString &fileName) const;  //!< Returns true if fileName is an acceptable file name for the current database
    void constructorSetup();                                //!< Helper method to consolidate common constructor initialization
    void finalizeReset();                                   //!< Finalizes the reset process
    QString installFileName() const;                        //!< Returns the filename to indicate that this database is currently being installed


    // ------------------------------------------------------------------------------------------------
    // Private members
    QString dbPrefix_;                      //!< Database prefix of desired database to download
    QUrl ncbiBlastDbUrl_;                   //!< Url of NCBI blast database directory
    QFtp *ftp_;                             //!< Ftp instance for downloading remote data
    GunzipPipe *gunzipPipe_;                //!< Decompression pipe
    UntarSink *untarSink_;                  //!< Untar output device

    QVector<TarBallFile> tarBalls_;         //!< List of ftp get requests for tarball files
    QHash<int, int> ftpCommandTarBallHash_; //!< Hash linking a ftp get command request to its index in the tarBalls_ vector; {ftp id -> tarBalls_ index}

    bool canceled_;                         //!< Boolean flag indicating cancel status
    bool resetting_;                        //!< Boolean flag indicating reset status
    bool otherError_;                       //!< Boolean flag indicating other error status (e.g. from gunzip or untar)
    QString otherErrorMessage_;             //!< Error message when an other error occurs (e.g. from gunzip or untar)
    RunState runState_;                     //!< The current state

    QHash<int, QString> ftpActionTexts_;    //!< Action messages to set as ftp commands are started
    QVector<QUrlInfo> cachedUrlInfos_;      //!< FTP directory listing cache
};


#endif // NCBIBLASTDATABASEINSTALLER_H
