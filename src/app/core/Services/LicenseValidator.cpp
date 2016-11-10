/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "LicenseValidator.h"
#include "../factories/LicenseFactory.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
bool LicenseValidator::isValidLicense(const QString &name, const QString &licenseKey)
{
    if (name.isEmpty() || licenseKey.isEmpty())
        return false;

    LicenseFactory licenseFactory;
    QByteArray licenseForName = licenseFactory.makeLicenseKey(name);
    QByteArray strippedLicenseKey = licenseFactory.stripFriendlyChars(licenseKey.toAscii());
    return licenseForName == strippedLicenseKey;
}
