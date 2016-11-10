/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSABUILDERFACTORY_H
#define MSABUILDERFACTORY_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include "../global.h"

class QStringList;

class AbstractMsaBuilder;
class ClustalWMsaBuilder;
class KalignMsaBuilder;

class MsaBuilderFactory : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    MsaBuilderFactory(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    AbstractMsaBuilder *makeMsaBuilder(const QString &msaBuilderId) const;
    QStringList supportedMsaBuilderIds() const;


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    ClustalWMsaBuilder *makeClustalWMsaBuilder() const;
    KalignMsaBuilder *makeKalignMsaBuilder() const;
    QString fullProgramPath(const QString &relativeProgramPath) const;
};

#endif // MSABUILDERFACTORY_H
