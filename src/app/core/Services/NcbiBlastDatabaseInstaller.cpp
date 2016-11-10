/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QMetaType>
#include <QtNetwork/QAbstractSocket>    // For communicating errors in a multithreaded environment
#include <QtNetwork/QFtp>

#include <QtDebug>

#include "NcbiBlastDatabaseInstaller.h"
#include "../util/GunzipPipe.h"
#include "../util/UntarSink.h"
#include "../macros.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param title [const QString &]
  * @param dbPrefix [const QString &]
  * @param parent [QObject *]
  */
NcbiBlastDatabaseInstaller::NcbiBlastDatabaseInstaller(const QString &title, QObject *parent)
    : AbstractBlastDatabaseInstaller(title, parent),
      ftp_(nullptr),
      gunzipPipe_(nullptr),
      untarSink_(nullptr),
      canceled_(false),
      resetting_(false),
      otherError_(false),
      runState_(StartState)
{
    constructorSetup();
}

/**
  * @param title [const QString &]
  * @param dbPrefix [const QString &]
  * @param parent [QObject *]
  */
NcbiBlastDatabaseInstaller::NcbiBlastDatabaseInstaller(const QString &title, const QString &dbPrefix, QObject *parent)
    : AbstractBlastDatabaseInstaller(title, parent),
      dbPrefix_(dbPrefix),
      ftp_(nullptr),
      gunzipPipe_(nullptr),
      untarSink_(nullptr),
      canceled_(false),
      resetting_(false),
      otherError_(false),
      runState_(StartState)
{
    constructorSetup();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QUrl
  */
QUrl NcbiBlastDatabaseInstaller::ncbiBlastDbUrl() const
{
    return ncbiBlastDbUrl_;
}

/**
  * @param url [const QUrl &]
  */
void NcbiBlastDatabaseInstaller::setNcbiBlastDbUrl(const QUrl &url)
{
    ASSERT(url.scheme() == "ftp");
    ncbiBlastDbUrl_ = url;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void NcbiBlastDatabaseInstaller::cancel()
{
    canceled_ = true;
    runState_ = ErrorState;

    if (ftp_->state() != QFtp::Unconnected)
    {
        ftp_->abort();

        // Do to the QFtp abort signal requiring an inordinate amount of time to finish, we simply delete the instance
        // to more quickly shut it down.
        delete ftp_;
        ftp_ = nullptr;
    }

    emit canceled();
}

/**
  * Removes any leftover files. Must be in an inactive state.
  */
void NcbiBlastDatabaseInstaller::cleanUp()
{
    QFile::remove(installFileName());

    foreach (const TarBallFile &tarBall, tarBalls_)
        foreach (const QString &file, tarBall.archivedFiles_)
            QFile::remove(file);
}

/**
  */
void NcbiBlastDatabaseInstaller::clearListCache()
{
    cachedUrlInfos_.clear();
}

/**
  */
void NcbiBlastDatabaseInstaller::reset()
{
    if (ftp_ != nullptr && ftp_->state() != QFtp::Unconnected)
    {
        resetting_ = true;
        if (ftp_->hasPendingCommands())
            ftp_->abort();
        ftp_->close();
    }
    else
    {
        finalizeReset();
    }
}

/**
  * @param directory [const QDir &]
  * @returns bool
  */
bool NcbiBlastDatabaseInstaller::setDestinationDirectory(const QDir &directory)
{
    if (isActive())
        return false;

    return untarSink_->setDirectory(directory) &&
           AbstractBlastDatabaseInstaller::setDestinationDirectory(directory);
}

/**
  * @param dbPrefix [const QString &]
  * @returns bool
  */
bool NcbiBlastDatabaseInstaller::setPrefix(const QString &dbPrefix)
{
    if (isActive())
        return false;

    dbPrefix_ = dbPrefix;

    return true;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void NcbiBlastDatabaseInstaller::execute()
{
    // Lazy initialization of the ftp instance serves not only the purpose of efficient data preparation, but also,
    // avoids another more subtle error. Specifically, Qftp creates additional children/instances. When this class
    // is created it originally instantiated the QFtp instance simultaneously. In a MT environment, the order goes:
    // x = new NcbiBlastDatabaseInstaller
    // --> new QFtp
    // x.moveToThread(...)
    //
    // If there is not enough processing time between the new QFtp and x.moveToThread, qftp will attempt to create
    // children and complain "Cannot create children for a parent that is in a different thread" Thus, by waiting until
    // the execute method, we virtually guarantee that it will be setup properly regardless of threading.
    if (ftp_ == nullptr)
    {
        ftp_ = new QFtp(this);

        connect(ftp_, SIGNAL(commandStarted(int)), SLOT(onFtpCommandStarted(int)));
        connect(ftp_, SIGNAL(commandFinished(int,bool)), SLOT(onFtpCommandFinished(int,bool)));
        connect(ftp_, SIGNAL(listInfo(QUrlInfo)), SLOT(onFtpListInfo(QUrlInfo)));
        connect(ftp_, SIGNAL(done(bool)), SLOT(onFtpDone(bool)));
        connect(ftp_, SIGNAL(dataTransferProgress(qint64,qint64)), SLOT(onFtpDataTransferProgress(qint64,qint64)));
    }

    if (runState_ == StartState)
    {
        if (ncbiBlastDbUrl_.isEmpty())
        {
            emit error("NCBI blast database url has not been defined");
            return;
        }
        else if (dbPrefix_.isEmpty())
        {
            emit error("No database prefix has been defined");
            return;
        }

        setActive(true);
        getFtpFileListing();
    }
#ifdef QT_DEBUG
    else
        qWarning("%s: not ready", Q_FUNC_INFO);
#endif
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param fileName [const QString &]
  */
void NcbiBlastDatabaseInstaller::onExtractStarted(const QString &fileName)
{
    if (ftp_->currentId() == 0)
        return;

    if (!ftpCommandTarBallHash_.contains(ftp_->currentId()))
        return;

    int i = ftpCommandTarBallHash_.value(ftp_->currentId());
    tarBalls_[i].archivedFiles_ << destinationDirectory().path() + QDir::separator() + fileName;
}

/**
  * @param ftpId [int]
  */
void NcbiBlastDatabaseInstaller::onFtpCommandStarted(int ftpId)
{
    if (ftpActionTexts_.contains(ftpId))
    {
        setActionText(ftpActionTexts_.take(ftpId));
        return;
    }

    if (!ftpCommandTarBallHash_.contains(ftpId))
        return;

    int i = ftpCommandTarBallHash_.value(ftpId);
    const TarBallFile &tarBallFile = tarBalls_.at(i);
    setActionText(QString("(%1 / %2) Downloading and unpacking: %3 (%4)")
                  .arg(i+1)
                  .arg(tarBalls_.size())
                  .arg(tarBallFile.ftpFile_.name())
                  .arg(tarBallFile.ftpFile_.size()));
}

/**
  * @param ftpId [int]
  * @param error [bool]
  */
void NcbiBlastDatabaseInstaller::onFtpCommandFinished(int ftpId, bool error)
{
    if (error)
    {
        if (!canceled_)
            ftp_->abort();
    }
    else
    {
        // If a ftp file transfer has completed, it is vital that the gunzipPipe be reset for the next incoming file.
        if (ftpCommandTarBallHash_.contains(ftpId))
            gunzipPipe_->reset();
    }
}

/**
  * @param done [qint64]
  * @param total [qint64]
  */
void NcbiBlastDatabaseInstaller::onFtpDataTransferProgress(qint64 done, qint64 /* total */)
{
    ASSERT(ftpCommandTarBallHash_.contains(ftp_->currentId()));

    // The total is bogus because it does not know the actual size. Thus, we use the size of the file when we first
    // located it.
    int i = ftpCommandTarBallHash_.value(ftp_->currentId());
    emit progressChanged(done, tarBalls_.at(i).ftpFile_.size());
}

/**
  * @param isError [bool]
  */
void NcbiBlastDatabaseInstaller::onFtpDone(bool isError)
{
    ftpActionTexts_.clear();

    if (canceled_)
    {
        emit canceled();
        return;
    }

    if (resetting_)
    {
        finalizeReset();
        return;
    }

    if (otherError_)
    {
        emit error(otherErrorMessage_);
        return;
    }

    // Otherwise assume that another type of error has occurred
    if (isError)
    {
        runState_ = ErrorState;
        emit error(ftp_->errorString());
        return;
    }

    switch (runState_)
    {
    case GetFileListState:
        foreach (const QUrlInfo &ftpFile, cachedUrlInfos_)
            if (acceptFtpFileName(ftpFile.name()))
                tarBalls_ << TarBallFile(ftpFile);

        downloadUnpack();
        break;
    case DownloadUnpackState:
        installSuccess();
        break;

    default:
        break;
    }
}

/**
  * Filter out all files except those that have the following traits:
  * o readable
  * o file
  * o Begin with the dbPrefix_ (must not be empty)
  * o ends with .tar.gz
  *
  * @param ftpFile [const QUrlInfo &]
  */
void NcbiBlastDatabaseInstaller::onFtpListInfo(const QUrlInfo &ftpFile)
{
    if (dbPrefix_.isEmpty())
        return;

    if (!ftpFile.isFile() || !ftpFile.isReadable())
        return;

    cachedUrlInfos_ << ftpFile;
}

/**
  * @param errorMessage [const QString &]
  */
void NcbiBlastDatabaseInstaller::onGunzipError(const QString &errorMessage)
{
    qDebug() << Q_FUNC_INFO << errorMessage;

    runState_ = ErrorState;
    otherError_ = true;
    otherErrorMessage_ = errorMessage;
    ftp_->abort();
}

/**
  * @param errorMessage [const QString &]
  */
void NcbiBlastDatabaseInstaller::onUntarError(const QString &errorMessage)
{
    qDebug() << Q_FUNC_INFO << errorMessage;

    runState_ = ErrorState;
    otherError_ = true;
    otherErrorMessage_ = errorMessage;
    ftp_->abort();
}

/**
  */
void NcbiBlastDatabaseInstaller::getFtpFileListing()
{
    runState_ = GetFileListState;

    setActionText("Initializing FTP connection to NCBI");

    int ftpId = ftp_->connectToHost(ncbiBlastDbUrl_.host());
    ftpActionTexts_.insert(ftpId, "Connecting to NCBI FTP site");
    ftpId = ftp_->login();
    ftpActionTexts_.insert(ftpId, "Logging in to NCBI FTP site");
    ftpId = ftp_->cd(ncbiBlastDbUrl_.path());
    ftpActionTexts_.insert(ftpId, "Opening blast database directory");
    if (cachedUrlInfos_.isEmpty())
    {
        ftpId = ftp_->list();
        ftpActionTexts_.insert(ftpId, "Retrieving database information");
    }

    // If an error occurs with any of the above, onFtpDone(false) will be called; otherwise, execution continues with
    // onFtpListInfo and finally onFtpDone(true).
}

/**
  */
void NcbiBlastDatabaseInstaller::downloadUnpack()
{
    runState_ = DownloadUnpackState;

    ASSERT(gunzipPipe_ != nullptr);
    ASSERT(ftp_ != nullptr);
//    ASSERT(ftp_->state() == QFtp::LoggedIn);

    if (tarBalls_.isEmpty())
    {
        runState_ = ErrorState;
        emit error(QString("BLAST database (%1) not found").arg(dbPrefix_));
        return;
    }

    // Create the ${dbPrefix_}.install file so that the BLAST database manager knows that this database is being
    // installed
    QFile installFile(installFileName());
    if (!installFile.open(QIODevice::WriteOnly))
    {
        runState_ = ErrorState;
        emit error(QString("Unable to write to output directory: %1").arg(destinationDirectory().path()));
        return;
    }
    installFile.close();

    setActionText("Downloading and unpacking");
    for (int i=0, z=tarBalls_.size(); i<z; ++i)
    {
        int ftpId = ftp_->get(tarBalls_.at(i).ftpFile_.name(), gunzipPipe_);
        ftpCommandTarBallHash_.insert(ftpId, i);
    }

    // Execution continues with onFtpCommandStarted(id) and onFtpDone(bool)
}

/**
  */
void NcbiBlastDatabaseInstaller::installSuccess()
{
    runState_ = StartState;
    QFile::remove(installFileName());
    reset();
    emit finished();
}



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Acceptable names are those that are simply the dbPrefix with a .tar.gz extension or those that have .\d\d in
  * between the prefix and extension.
  *
  * @param fileName [const QString &]
  * @returns bool
  */
bool NcbiBlastDatabaseInstaller::acceptFtpFileName(const QString &fileName) const
{
    if (fileName == dbPrefix_ + ".tar.gz")
        return true;

    // + 3 characters for .\d\d
    // + 7 = length of .tar.gz
    if (fileName.length() == dbPrefix_.length() + 3 + 7 &&
        fileName.startsWith(dbPrefix_) &&
        fileName.endsWith(".tar.gz") &&
        fileName.at(dbPrefix_.length()) == '.' &&
        fileName.at(dbPrefix_.length()+1).isDigit() &&
        fileName.at(dbPrefix_.length()+2).isDigit())
    {
        return true;
    }

    return false;
}

/**
  */
void NcbiBlastDatabaseInstaller::constructorSetup()
{
    untarSink_ = new UntarSink(this);
    connect(untarSink_, SIGNAL(error(QString)), SLOT(onUntarError(QString)));
    connect(untarSink_, SIGNAL(extractStarted(QString)), SLOT(onExtractStarted(QString)));

    gunzipPipe_ = new GunzipPipe(untarSink_);
    connect(gunzipPipe_, SIGNAL(error(QString)), SLOT(onGunzipError(QString)));

    // Necessary to use QUrlInfo in separate thread
    qRegisterMetaType<QUrlInfo>("QUrlInfo");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
}

/**
  */
void NcbiBlastDatabaseInstaller::finalizeReset()
{
    tarBalls_.clear();
    ftpCommandTarBallHash_.clear();

    gunzipPipe_->reset();
    untarSink_->reset();

    setActive(false);

    canceled_ = false;
    resetting_ = false;
    otherError_ = false;
    otherErrorMessage_.clear();

    runState_ = StartState;
}

/**
  * @returns QString
  */
QString NcbiBlastDatabaseInstaller::installFileName() const
{
    return destinationDirectory().path() + QDir::separator() + dbPrefix_ + ".install";
}
