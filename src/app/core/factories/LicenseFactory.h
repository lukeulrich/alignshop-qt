/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LICENSEFACTORY_H
#define LICENSEFACTORY_H

#include <QtCore/QByteArray>

class LicenseFactory
{
public:
    QByteArray makeLicenseKey(const QString &name) const;
    QByteArray makeFriendlyLicenseKey(const QString &name) const;
    QByteArray stripFriendlyChars(const QByteArray &friendlyLicenseKey) const;

private:
    QString normalizeName(const QString &name) const;
    QByteArray hashName(const QString &name) const;
};

#endif // LICENSEFACTORY_H
