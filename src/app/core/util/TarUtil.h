/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TARUTIL_H
#define TARUTIL_H

#include "../PODs/TarHeaderPod.h"

/**
  * TarUtil is a helper class for common TAR file format tasks.
  *
  * ISSUES:
  * o prefix is not used
  * o If a field exceeds its designated field length, it will be truncated
  */
class TarUtil
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    TarHeaderPod header(const QString &file) const;             //!< Creates a ustar-type TarHeaderPod for file
    TarHeaderPod toPod(const TarHeaderRawPod &rawPod) const;    //!< Converts rawPod into its TarHeaderPod equivalent
    TarHeaderRawPod toRawPod(const TarHeaderPod &pod) const;    //!< Converts pod into its TarHeaderRawPod equivalent
    bool validChecksum(const TarHeaderRawPod &rawPod) const;    //!< Returns true if rawPod contains a valid checksum; false otherwise

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! Computes the checksum for rawPod using signed chars
    qint64 computeSignedChecksum(const TarHeaderRawPod &rawPod) const;
    //! Computes the checksum for rawPod using unsigned chars
    qint64 computeUnsignedChecksum(const TarHeaderRawPod &rawPod) const;
    //! Returns a copy of byteArray with any leading zeroes removed
    QByteArray removeLeadingZeroes(const QByteArray &byteArray) const;
};

#endif // TARUTIL_H
