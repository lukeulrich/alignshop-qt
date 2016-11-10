/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <qplatformdefs.h>      // For stat'ing symlinks ourselves. Should be the very first include file

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "TarUtil.h"
#include "../global.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Throws an exception if file does not exist. Currently supports the following file types:
  * o Normal files
  * o Directories
  * o Symlinks
  *
  * Hard links and contiguous files are not distinguished from normal files and will have a normal file type flag.
  *
  * Only captures the read/write/execute permissions for owner/group/other. Setuid and related bits are ignored.
  *
  * Symlinks always have full permissions
  *
  * The inclusion of named pipes (FIFO) is undefined and currently not supported.
  *
  * The filename for directories is always terminated with a directory separator. Symlinks it is not.
  *
  * TODO: bundles on MacOSX
  *
  * @param file [const QString &]
  * @returns TarHeaderPod
  */
TarHeaderPod TarUtil::header(const QString &file) const
{
    QFileInfo fileInfo(file);
    if (!fileInfo.exists())
        throw QString("File does not exist: %1").arg(file);

    if (!fileInfo.isDir() && !fileInfo.isFile())
        throw QString("Unrecognized file type. Only files, directories, and symlinks supported");

    TarHeaderPod pod;
    pod.fileName_ = QDir::current().relativeFilePath(fileInfo.filePath()).toAscii();
    if (fileInfo.isDir() && !fileInfo.isSymLink())
        pod.fileName_ += QDir::separator();
    if (fileInfo.isSymLink())
    {
        pod.permissions_ = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                           QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                           QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                           QFile::ReadOther | QFile::WriteOther | QFile::ExeOther;
    }
    else
    {
        pod.permissions_ = fileInfo.permissions();
    }
    pod.uid_ = fileInfo.ownerId();
    pod.gid_ = fileInfo.groupId();
    if (fileInfo.isFile())
        pod.fileSize_ = fileInfo.size();


    // Last modification time
    // This one is tricky. QFileInfo gives us the last modified time of the target if it is a symlink; yet, for tar
    // purposes, we want the last modified time of the symlink itself. QFileInfo does not provide such info, therefore
    // we have to do it using QT's cross-platform stat capabilities.
    if (fileInfo.isSymLink())
    {
        QT_STATBUF stat;
#ifdef Q_OS_UNIX
        QT_LSTAT(file.toAscii(), &stat);
#else
        QT_STAT(file.toAscii(), &stat);
#endif
        pod.lastModificationTime_ = QDateTime::fromMSecsSinceEpoch(stat.st_mtime * 1000);
    }
    else
    {
        pod.lastModificationTime_ = fileInfo.lastModified();
    }
    pod.linkType_ = (fileInfo.isSymLink()) ? eSymLinkType :
                                             (fileInfo.isDir()) ? eDirectoryType :
                                                                  eFileType;
    if (fileInfo.isSymLink())
        pod.linkName_ = fileInfo.dir().relativeFilePath(fileInfo.symLinkTarget()).toAscii();

    pod.ustar_ = true;
    pod.userName_ = fileInfo.owner().toAscii();
    pod.groupName_ = fileInfo.group().toAscii();

    return pod;
}

/**
  * @param rawPod [const TarHeaderRawPod &]
  * @returns TarHeaderPod
  */
TarHeaderPod TarUtil::toPod(const TarHeaderRawPod &rawPod) const
{
    if (!validChecksum(rawPod))
        throw QString("Invalid header - checksum mismatch");

    TarHeaderPod pod;

    pod.fileName_ = rawPod.fileName_;

    // Permissions
    enum {
        READ = 4,
        WRITE = 2,
        EXEC = 1
    };

    // User + Owner
    int perm = rawPod.permissions_[4];
    if (perm & READ)
        pod.permissions_ |= QFile::ReadUser | QFile::ReadOwner;
    if (perm & WRITE)
        pod.permissions_ |= QFile::WriteUser | QFile::WriteOwner;
    if (perm & EXEC)
        pod.permissions_ |= QFile::ExeUser | QFile::ExeOwner;

    perm = rawPod.permissions_[5];
    if (perm & READ)
        pod.permissions_ |= QFile::ReadGroup;
    if (perm & WRITE)
        pod.permissions_ |= QFile::WriteGroup;
    if (perm & EXEC)
        pod.permissions_ |= QFile::ExeGroup;

    perm = rawPod.permissions_[6];
    if (perm & READ)
        pod.permissions_ |= QFile::ReadOther;
    if (perm & WRITE)
        pod.permissions_ |= QFile::WriteOther;
    if (perm & EXEC)
        pod.permissions_ |= QFile::ExeOther;

    // Uid
    QByteArray buffer = removeLeadingZeroes(rawPod.uid_);
    pod.uid_ = buffer.toInt(nullptr, 8);

    // Gid
    buffer = removeLeadingZeroes(rawPod.gid_);
    pod.gid_ = buffer.toInt(nullptr, 8);

    // Filesize
    buffer = removeLeadingZeroes(rawPod.fileSize_);
    pod.fileSize_ = buffer.toULong(nullptr, 8);

    // Last modification time
    buffer = removeLeadingZeroes(rawPod.lastModificationTime_);
    qint64 secsSinceEpoch = buffer.toULong(nullptr, 8);
    pod.lastModificationTime_ = QDateTime::fromMSecsSinceEpoch(secsSinceEpoch * 1000);

    // linktype
    char tmp[2] = {0};
    tmp[0] = rawPod.linkType_;
    pod.linkType_ = static_cast<LinkType>(QByteArray(tmp).toInt());

    // linkname
    pod.linkName_ = rawPod.linkName_;

    // ustar
    pod.ustar_ = rawPod.ustar_[0] == 'u' &&
                 rawPod.ustar_[1] == 's' &&
                 rawPod.ustar_[2] == 't' &&
                 rawPod.ustar_[3] == 'a' &&
                 rawPod.ustar_[4] == 'r' &&
                 rawPod.ustar_[5] == ' ';

    // Do nothing with version

    // user name
    pod.userName_ = rawPod.userName_;

    // group name
    pod.groupName_ = rawPod.groupName_;

    // devMajorNumber
    buffer = removeLeadingZeroes(rawPod.devMajorNumber_);
    if (buffer.length())
        // ISSUE: Using base 10 here....
        pod.devMajorNumber_ = buffer.toInt(nullptr);

    // devMinorNumber
    buffer = removeLeadingZeroes(rawPod.devMinorNumber_);
    if (buffer.length())
        // ISSUE: Using base 10 here....
        pod.devMinorNumber_ = buffer.toInt(nullptr);

    pod.prefix_ = rawPod.prefix_;

    return pod;
}

/**
  * Creates unsigned checksums.
  * Permissions for symlinks are always 777.
  *
  * @param pod [const TarHeaderPod &]
  * @returns TarHeaderRawPod
  */
TarHeaderRawPod TarUtil::toRawPod(const TarHeaderPod &pod) const
{
    TarHeaderRawPod rawPod;

#ifdef Q_OS_WIN
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

    // Filename
    memcpy(rawPod.rawData_, pod.fileName_.constData(), pod.fileName_.length());

    // Permissions (originally in hexadecimal)
    QByteArray buffer;
    memcpy(rawPod.rawData_ + 100, "0000", 4);
    buffer.setNum((pod.permissions_ & (QFile::ReadUser | QFile::WriteUser | QFile::ExeUser)) >> 8);
    rawPod.rawData_[100 + 4] = buffer.at(0);
    buffer.setNum((pod.permissions_ & (QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup)) >> 4);
    rawPod.rawData_[100 + 5] = buffer.at(0);
    buffer.setNum(pod.permissions_ & (QFile::ReadOther | QFile::WriteOther | QFile::ExeOther));
    rawPod.rawData_[100 + 6] = buffer.at(0);

    // User id
    buffer = buffer.setNum(pod.uid_, 8);
    memcpy(rawPod.rawData_ + 108, buffer.rightJustified(7, '0').constData(), 7);

    // Group id
    buffer.setNum(pod.gid_, 8);
    memcpy(rawPod.rawData_ + 116, buffer.rightJustified(7, '0').constData(), 7);

    // File size
    int writeSize = (pod.linkType_ == eFileType) ? pod.fileSize_ : 0;
    buffer.setNum(writeSize, 8);
    memcpy(rawPod.rawData_ + 124, buffer.rightJustified(11, '0').constData(), 11);

    // Last modification time
    // This one is tricky. QFileInfo gives us the last modified time of the target if it is a symlink; yet, for tar
    // purposes, we want the last modified time of the symlink itself. QFileInfo does not provide such info, therefore
    // we have to do it using QT's cross-platform stat capabilities.
    if (pod.linkType_ == eSymLinkType)
    {
        QT_STATBUF stat;
#ifdef Q_OS_UNIX
        QT_LSTAT(pod.fileName_, &stat);
#else
        QT_STAT(pod.fileName_, &stat);
#endif
        buffer.setNum(static_cast<int>(stat.st_mtime), 8);
    }
    else
    {
        buffer.setNum(pod.lastModificationTime_.toMSecsSinceEpoch() / 1000, 8);
    }
    memcpy(rawPod.rawData_ + 136, buffer.rightJustified(11, '0').constData(), 11);

    // Checksum is done as a final step

    // Link type
    buffer.setNum(pod.linkType_);
    rawPod.rawData_[156] = buffer.at(0);

    // linkName
    memcpy(rawPod.rawData_ + 157, pod.linkName_.constData(), qMin(100, pod.linkName_.length()));

    // ustar fields
    if (pod.ustar_)
    {
        // Ustar field
        memcpy(rawPod.rawData_ + 257, "ustar ", 6);

        // version
        memcpy(rawPod.rawData_ + 263, " \0", 2);

        // user name
        memcpy(rawPod.rawData_ + 265, pod.userName_.constData(), qMin(32, pod.userName_.length()));

        // group name
        memcpy(rawPod.rawData_ + 297, pod.groupName_.constData(), qMin(32, pod.groupName_.length()));

        if (pod.linkType_ == eBlockSpecialType ||
            pod.linkType_ == eCharSpecialType)
        {
            // dev major number
            buffer.setNum(pod.devMajorNumber_, 8);
            memcpy(rawPod.rawData_ + 329, buffer.rightJustified(7, '0'), 7);

            // dev minor number
            buffer.setNum(pod.devMinorNumber_, 8);
            memcpy(rawPod.rawData_ + 337, buffer.rightJustified(7, '0'), 7);
        }

        // prefix
        memcpy(rawPod.rawData_ + 345, pod.prefix_.constData(), qMin(155, pod.prefix_.length()));
    }

    // Finally the checksum
    buffer.setNum(computeUnsignedChecksum(rawPod), 8);
    memcpy(rawPod.rawData_ + 148, buffer.rightJustified(6, '0').constData(), 6);
    rawPod.rawData_[148 + 7] = ' ';

#ifdef Q_OS_WIN
#pragma warning(pop)
#endif

    return rawPod;
}

/**
  * @param rawPod [const TarHeaderRawPod &]
  * @returns bool
  */
bool TarUtil::validChecksum(const TarHeaderRawPod &rawPod) const
{
    QByteArray array(removeLeadingZeroes(rawPod.checksum_));

    bool ok = false;
    qint64 checksum = array.toInt(&ok, 8);
    if (!ok)
        return false;

    return checksum == computeUnsignedChecksum(rawPod) ||
           checksum == computeSignedChecksum(rawPod);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param rawPod [const TarHeaderRawPod &]
  * @returns qint64
  */
qint64 TarUtil::computeSignedChecksum(const TarHeaderRawPod &rawPod) const
{
    qint64 checksum = 0;

    const char *x = rawPod.rawData_;
    for (int i=0; i< kTarHeaderSize; ++x, ++i)
        checksum += *x;

    // Discount the actual checksum values and compute the value for spaces
    x = rawPod.rawData_ + 148;
    for (int i=0; i< 8; ++i, ++x)
    {
        checksum -= *x;
        checksum += ' ';
    }

    return checksum;
}

/**
  * @param rawPod [const TarHeaderRawPod &]
  * @returns qint64
  */
qint64 TarUtil::computeUnsignedChecksum(const TarHeaderRawPod &rawPod) const
{
    qint64 checksum = 0;

    const uchar *x = (uchar *)rawPod.rawData_;
    for (int i=0; i< kTarHeaderSize; ++x, ++i)
        checksum += *x;

    // Discount the actual checksum values and compute the value for spaces
    x = (uchar *)rawPod.rawData_ + 148;
    for (int i=0; i< 8; ++i, ++x)
    {
        checksum -= *x;
        checksum += ' ';
    }

    return checksum;
}

/**
  * @param byteArray [const QByteArray &]
  * @returns QByteArray
  */
QByteArray TarUtil::removeLeadingZeroes(const QByteArray &byteArray) const
{
    QByteArray result = byteArray;

    const char *x = byteArray.constData();
    int nZeroes = 0;
    while (*x == '0')
    {
        ++nZeroes;
        ++x;
    }

    result.remove(0, nZeroes);

    return result;
}
