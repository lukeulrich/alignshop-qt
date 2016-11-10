/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCryptographicHash>

#include "LicenseFactory.h"
#include "../constants.h"
#include "../../version.h"

static const QByteArray kInsideSecret = "nhf0*lTcl!9d-;;";

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
QByteArray LicenseFactory::makeLicenseKey(const QString &name) const
{
    QString normalizedName = normalizeName(name);
    QByteArray fullLicenseKey = QCryptographicHash::hash(constants::kOutsideSecret +
                                                         kInsideSecret +
                                                         QByteArray::number(constants::version::kMajorVersion) +
                                                         constants::version::kBuildType +
                                                         normalizedName.toAscii(),
                                                         QCryptographicHash::Md5);
    QByteArray partialLicenseKey = fullLicenseKey.toHex().left(16).toUpper();

    // Convert easily mistaken characters
    partialLicenseKey.replace("0", "Z");
    partialLicenseKey.replace("G", "J");
    partialLicenseKey.replace("B", "Q");

    return partialLicenseKey;
}

QByteArray LicenseFactory::makeFriendlyLicenseKey(const QString &name) const
{
    QByteArray licenseKey = makeLicenseKey(name);
    QByteArray friendlyLicenseKey = licenseKey.mid(0, 4) + '-' +
                                    licenseKey.mid(4, 4) + '-' +
                                    licenseKey.mid(8, 4) + '-' +
                                    licenseKey.mid(12, 4);
    return friendlyLicenseKey;
}

QByteArray LicenseFactory::stripFriendlyChars(const QByteArray &friendlyLicenseKey) const
{
    QByteArray strippedLicense = friendlyLicenseKey;
    strippedLicense.replace("-", "").replace(" ", "");
    return strippedLicense;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
QString LicenseFactory::normalizeName(const QString &name) const
{
    return name.trimmed();
}
