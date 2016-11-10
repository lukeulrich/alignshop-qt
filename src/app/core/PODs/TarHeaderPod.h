/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TARHEADERPOD_H
#define TARHEADERPOD_H

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QString>

static const int kTarHeaderSize = 512;

enum LinkType
{
    eFileType = 0,                  //!< Standard file
    eHardLinkType,                  //!< Hard link
    eSymLinkType,                   //!< Symbolic link (*nix), shortcut on windows
    eCharSpecialType,               //!< Character device
    eBlockSpecialType,              //!< Block device
    eDirectoryType,                 //!< Directory
    eFIFOType,                      //!< Named pipe or FIFO
    eContiguousType
};

/**
  * TarHeaderPod is a Qt object-friendly representation of a TAR header.
  */
struct TarHeaderPod
{
    // ------------------------------------------------------------------------------------------------
    // Public members
    QByteArray fileName_;
    QFile::Permissions permissions_;
    uint uid_;
    uint gid_;
    qint64 fileSize_;
    QDateTime lastModificationTime_;
    LinkType linkType_;
    QByteArray linkName_;

    // Ustar fields
    bool ustar_;
    int version_;
    QByteArray userName_;
    QByteArray groupName_;
    uint devMajorNumber_;
    uint devMinorNumber_;
    QByteArray prefix_;


    // ------------------------------------------------------------------------------------------------
    // Constructor
    /**
      * Initialize all variables to zero.
      */
    TarHeaderPod()
    {
        permissions_ = 0;
        uid_ = 0;
        gid_ = 0;
        linkType_ = eFileType;
        fileSize_ = 0;
        ustar_ = false;
        version_ = 0;
        devMajorNumber_ = 0;
        devMinorNumber_ = 0;
    }


    // ------------------------------------------------------------------------------------------------
    // Operators
    /**
      * Returns true if other is equivalent to this instance.
      *
      * @param other [const TarHeaderPod &]
      * @returns bool
      */
    bool operator==(const TarHeaderPod &other) const
    {
        // Test for self
        if (this == &other)
            return true;

        return fileName_ == other.fileName_ &&
                permissions_ == other.permissions_ &&
                uid_ == other.uid_ &&
                gid_ == other.gid_ &&
                fileSize_ == other.fileSize_ &&
                lastModificationTime_ == other.lastModificationTime_ &&
                linkType_ == other.linkType_ &&
                linkName_ == other.linkName_ &&
                ustar_ == other.ustar_ &&
                version_ == other.version_ &&
                userName_ == other.userName_ &&
                groupName_ == other.groupName_ &&
                devMajorNumber_ == other.devMajorNumber_ &&
                devMinorNumber_ == other.devMinorNumber_ &&
                prefix_ == other.prefix_;
    }

    /**
      * Returns true if other is not equivalent to this instance.
      *
      * @param other [const TarHeaderPod &]
      * @returns bool
      */
    bool operator!=(const TarHeaderPod &other) const
    {
        return !operator==(other);
    }
};
Q_DECLARE_TYPEINFO(TarHeaderPod, Q_MOVABLE_TYPE);



/**
  * TarHeaderRawPod maps a 512-byte TAR header to its character equivalents using a union and specific fields.
  *
  * Upon construction, it is initialized with ASCII NULL (0) bytes.
  */
// Integrates the GNU tar format and USTAR formats in a common structure
union TarHeaderRawPod
{
    char rawData_[kTarHeaderSize];
    struct {
        char fileName_[100];            // 0
        char permissions_[8];           // 100
        char uid_[8];                   // 108
        char gid_[8];                   // 116
        char fileSize_[12];             // 124 - all zeroes for directory and symlinks
        char lastModificationTime_[12]; // 136
        char checksum_[8];              // 148
        char linkType_;                 // 156
        // If linkType_ is a symlink, this field contains the file pointed to
        char linkName_[100];            // 157

        // Additional USTAR fields
        char ustar_[6];                 // 257
        char version_[2];               // 263: POSIX: two ASCII zeroes; or: " \0" (space followed by null)
        char userName_[32];             // 265
        char groupName_[32];            // 297
        char devMajorNumber_[8];        // 329
        char devMinorNumber_[8];        // 337
        char prefix_[155];              // 345
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor
    /**
      * Constructor that initializes all char data to NULL
      */
    TarHeaderRawPod()
    {
        // Zero out the array
        memset(rawData_, 0, sizeof(rawData_));
    }


    // ------------------------------------------------------------------------------------------------
    // Public methods
    /**
      * Returns true if all character data is NULL; false otherwise
      *
      * @returns bool
      */
    bool isNull() const
    {
        for (int i=0; i< kTarHeaderSize; ++i)
            if (rawData_[i] != 0)
                return false;

        return true;
    }
};
Q_DECLARE_TYPEINFO(TarHeaderRawPod, Q_MOVABLE_TYPE);

#endif // TARHEADERPOD_H
