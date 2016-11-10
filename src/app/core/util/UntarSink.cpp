/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "UntarSink.h"
#include "TarUtil.h"
#include "../macros.h"

static const int kNumberOfTicks = 100;   // Maximum number of progressChanged signals to emit per file

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
UntarSink::UntarSink(QObject *parent)
    : QIODevice(parent),
      errored_(false),
      tarHeaderBytesRead_(0),
      skipBytes_(0)
{
    open(QIODevice::WriteOnly);
}

/**
  * @param directory [const QDir &]
  * @param parent [QObject *]
  */
UntarSink::UntarSink(const QDir &directory, QObject *parent)
    : QIODevice(parent),
      directory_(directory),
      errored_(false),
      tarHeaderBytesRead_(0),
      skipBytes_(0)
{
    open(QIODevice::WriteOnly);
}

/**
  */
UntarSink::~UntarSink()
{
    reset();
    close();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QDir
  */
QDir UntarSink::directory() const
{
    return directory_;
}

/**
  * @returns bool
  */
bool UntarSink::isSequential()
{
    return true;
}

/**
  * Does not change the output directory.
  *
  * @returns bool
  */
bool UntarSink::reset()
{
    file_.close();
    file_.setFileName(QString());
    errored_ = false;
    setErrorString(QString());
    fileProgress_.written_ = 0;
    fileProgress_.total_ = 0;
    tarHeaderBytesRead_ = 0;
    skipBytes_ = 0;

    return QIODevice::reset();
}

/**
  * @param directory [const QDir &]
  * @returns bool
  */
bool UntarSink::setDirectory(const QDir &directory)
{
    if (!directory.exists())
        return false;

    directory_ = directory;

    return true;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Reimplemented to hide from public interface.
  */
void UntarSink::close()
{
    QIODevice::close();
}

/**
  * Reimplemented to hide from public interface.
  *
  * @param mode [OpenMode]
  * @returns bool
  */
bool UntarSink::open(OpenMode mode)
{
    return QIODevice::open(mode);
}

/**
  * @param data [char *]
  * @param maxlen [qint64]
  * @returns qint64
  */
qint64 UntarSink::readData(char * /* data */, qint64 /* maxlen */)
{
    setErrorString("Reading is not allowed from this device");
    return -1;
}

/**
  * Core function responsible for extracting a tar ball.
  *
  * Because each file's contents are rounded to the next multiple of 512 with any extra space zero padded, it may be
  * necessary to skip these zero bytes. This amount changes from file to file and thus is tracked via the skipBytes_
  * member.
  *
  * @param data [const char *]
  * @param len [qint64]
  * @returns qint64
  */
qint64 UntarSink::writeData(const char *data, qint64 len)
{
    if (errored_)
        return -1;

    static qint64 bytesPerProgressSignal = 0;     // Rough number of bytes per progress signal
    static qint64 signalByteThreshold = 0;

    qint64 i = 0;
    while (i != len)
    {
        const char *x = data + i;
        int remainingBytes = len - i;

        // Case 1: File is open
        if (file_.isOpen())
        {
            // Case 1.1: All bytes go to file
            if (remainingBytes + fileProgress_.written_ < fileProgress_.total_)
            {
                if (!writeAll(&file_, x, remainingBytes))
                    return -1;

                fileProgress_.written_ += remainingBytes;

                // Progress signal
                if (fileProgress_.written_ > signalByteThreshold)
                {
                    emit progressChanged(fileProgress_.written_, fileProgress_.total_);
                    // Compute when the next progress changed signal should be emitted
                    signalByteThreshold += (1 + (fileProgress_.written_ - signalByteThreshold) / bytesPerProgressSignal)
                                           * bytesPerProgressSignal;
                }

                return len;
            }

            // Case 1.2: There is at least enough data to finish writing the file
            int unwrittenFileBytes = fileProgress_.total_ - fileProgress_.written_;
            if (!writeAll(&file_, x, unwrittenFileBytes))
                return -1;

            i += unwrittenFileBytes;

            // Update the skip bytes as necessary
            if (fileProgress_.total_ % 512 != 0)
            {
                // This formula finds the next larget multiple of 512 that is greater than fileProgress_.total_
                skipBytes_ = (((fileProgress_.total_ / 512) + 1) * 512) - fileProgress_.total_;
            }

            // Close out the file
            file_.close();
            file_.setFileName(QString());
            emit progressChanged(fileProgress_.total_, fileProgress_.total_);
            emit extractFinished(file_.fileName());

            continue;
        }

        // Case 2: Do we need to skip any zero-padded bytes
        if (skipBytes_ > 0)
        {
            // Case 2.1: Not enough bytes in buffer to surpass all skip bytes
            if (remainingBytes < skipBytes_)
            {
                skipBytes_ -= remainingBytes;
                return len;
            }

            // Case 2.2: At least enough bytes to skip all the necessary bytes (remainingBytes >= skipBytes)
            i += skipBytes_;
            skipBytes_ = 0;
            continue;
        }

        // Case 3: No open file
        if (tarHeaderBytesRead_ + remainingBytes < kTarHeaderSize)
        {
            // Case 3.1: Save this up for the tar header
            memcpy(tarHeaderRawPod_.rawData_ + tarHeaderBytesRead_, x, remainingBytes);
            tarHeaderBytesRead_ += remainingBytes;
            return len;
        }

        // Case 3.2: At least enough data to finish a complete tar header
        int unwrittenHeaderBytes = kTarHeaderSize - tarHeaderBytesRead_;
        memcpy(tarHeaderRawPod_.rawData_ + tarHeaderBytesRead_, x, unwrittenHeaderBytes);
        i += unwrittenHeaderBytes;

        // Clear out the raw pod read bytes in order to provide for the next one; even though this variable is set here,
        // the current raw data is not changed until after the header pod has been processed first (see below).
        tarHeaderBytesRead_ = 0;

        // Ignore all null header pods (e.g. EOF markers)
        if (tarHeaderRawPod_.isNull())
            continue;

        try
        {
            TarUtil tarUtil;
            TarHeaderPod tarPod = tarUtil.toPod(tarHeaderRawPod_);
            if (!processTarHeaderPod(tarPod))
            {
                emit error(errorString());
                errored_ = true;
                return -1;
            }

            // Configure the number of progress signals
            if (tarPod.linkType_ == eFileType)
            {
                bytesPerProgressSignal = tarPod.fileSize_ / kNumberOfTicks;
                signalByteThreshold = bytesPerProgressSignal;
            }
        }
        catch (QString &errorMessage)
        {
            errored_ = true;
            emit error(errorMessage);
            setErrorString(errorMessage);
            return -1;
        }
    }

    return len;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param pod [const TarHeaderPod &]
  * @returns bool
  */
bool UntarSink::processTarDirectory(const TarHeaderPod &pod)
{
    // Relative to the output directory
    QString absoluteFilePath = directory_.absoluteFilePath(pod.fileName_);
    if (pod.fileSize_ > 0)
    {
        setErrorString(QString("Invalid tar header - directory (%1)) has positive file size").arg(QString(pod.fileName_)));
        return false;
    }

    // Build the relative directory
    QFileInfo fileInfo(absoluteFilePath);
    if (!fileInfo.exists())
    {
        if (!directory_.mkpath(absoluteFilePath))
        {
            setErrorString(QString("Unable to create directory (%1)").arg(absoluteFilePath));
            return false;
        }
    }
    // Else - the "file" (it may be a directory or something else) already exists. Make sure it is a directory
    else if (!fileInfo.isDir() || fileInfo.isSymLink())
    {
        setErrorString(QString("Unable to create directory (%1): a file already exists with that name").arg(QString(pod.fileName_)));
        return false;
    }

    // Attempt to update the permissions for the directory
    if (fileInfo.permissions() != pod.permissions_)
        QFile(absoluteFilePath).setPermissions(pod.permissions_);

    return true;
}

/**
  * @param pod [const TarHeaderPod &]
  * @returns bool
  */
bool UntarSink::processTarFile(const TarHeaderPod &pod)
{
    // Relative to the output directory
    QString absoluteFilePath = directory_.absoluteFilePath(pod.fileName_);
    file_.setFileName(absoluteFilePath);
    if (!file_.open(QIODevice::WriteOnly))  // Note this will overwrite any existing file!
    {
        setErrorString(file_.errorString());
        return false;
    }

    // ISSUE? Ignore whether setting the permissions succeeded or not
    file_.setPermissions(pod.permissions_);

    // Initialize the file progress members
    fileProgress_.written_ = 0;
    fileProgress_.total_ = pod.fileSize_;

    emit extractStarted(pod.fileName_);
    emit progressChanged(0, pod.fileSize_);
    if (pod.fileSize_ == 0)
    {
        // Empty file - go ahead and finish right now
        file_.setFileName(QString());
        file_.close();
        emit extractFinished(pod.fileName_);
    }

    return true;
}

/**
  * The following requirements must be met for this method to return successfully:
  * o output directory must exist and be writable by the current user
  * o Every header must have a non-empty filename
  * o linktype must be a file, directory, or symlink
  *
  * If a file already exists with the same name it will be overwritten. If a directory exists with the same name, it
  * will simply attempt to update the permissions, yet this does not trigger an error.
  *
  * @param pod [const TarHeaderPod &]
  * @returns bool
  */
bool UntarSink::processTarHeaderPod(const TarHeaderPod &pod)
{
    ASSERT(file_.isOpen() == false);

    // Double check that directory exists and is valid
    if (!directory_.exists())
    {
        setErrorString(QString("Output directory, %1, does not exist").arg(directory_.path()));
        return false;
    }

    if (pod.fileName_.isEmpty())
    {
        setErrorString("Invalid tar header - missing file name");
        return false;
    }

    switch(pod.linkType_)
    {
    case eFileType:
        return processTarFile(pod);
    case eDirectoryType:
        return processTarDirectory(pod);
    case eSymLinkType:
        return processTarSymlink(pod);

    default:
        setErrorString(QString("Unhandled link type (%1)").arg(QString(pod.fileName_)));
        return false;
    }
}

/**
  * @param pod [const TarHeaderPod &]
  * @returns bool
  */
bool UntarSink::processTarSymlink(const TarHeaderPod &pod)
{
    if (pod.linkName_.isEmpty())
    {
        setErrorString(QString("Invalid tar header - missing link name for file, %1").arg(QString(pod.fileName_)));
        return false;
    }

    if (pod.fileSize_ > 0)
    {
        setErrorString(QString("Invalid tar header - symlink (%1) has positive file size").arg(QString(pod.linkName_)));
        return false;
    }

    QString absoluteFilePath = directory_.absoluteFilePath(pod.fileName_);
    QFile::remove(absoluteFilePath);

    // With symlinks, the naming is kind of funky: fileName_ is actually the name for the symlink, and linkName_ is the
    // name of the file pointed to by this symlink.
    if (!QFile::link(directory_.relativeFilePath(pod.linkName_), directory_.absoluteFilePath(pod.fileName_)))
    {
        setErrorString(QString("Unable to create link (%1)").arg(QString(pod.linkName_)));
        return false;
    }

    return true;
}

/**
  * @param device [QIODevice *]
  * @param data [const char *]
  * @param length [int]
  * @returns bool
  */
bool UntarSink::writeAll(QIODevice *device, const char *data, int length)
{
    ASSERT(device != nullptr);

    int bytesWritten = 0;
    while (bytesWritten != length)
    {
        bytesWritten = device->write(data + bytesWritten, length - bytesWritten);
        if (bytesWritten == -1)
        {
            if (file_.isOpen())
                file_.close();

            errored_ = true;
            setErrorString("Device write error: " + device->errorString());
            emit error(errorString());
            return false;
        }
    }

    return true;
}
