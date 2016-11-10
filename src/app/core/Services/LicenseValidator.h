/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LICENSEVALIDATOR_H
#define LICENSEVALIDATOR_H

#include <QtCore/QByteArray>
#include <QtCore/QString>

class LicenseValidator
{
public:
    static bool isValidLicense(const QString &name, const QString &licenseKey);
};

#endif // LICENSEVALIDATOR_H
