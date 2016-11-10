/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QStringList>

#include "MsaBuilderFactory.h"
#include "../constants.h"
#include "../constants/MsaBuilderIds.h"
#include "../Services/KalignMsaBuilder.h"
#include "../Services/ClustalWMsaBuilder.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
MsaBuilderFactory::MsaBuilderFactory(QObject *parent)
    : QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
AbstractMsaBuilder *MsaBuilderFactory::makeMsaBuilder(const QString &msaBuilderId) const
{
    using namespace constants::MsaBuilder;
    if (msaBuilderId == kClustalWBuilderId)
        return makeClustalWMsaBuilder();
    else if (msaBuilderId == kKalignBuilderId)
        return makeKalignMsaBuilder();

    throw QString("Msa builder id, %1, not supported.").arg(msaBuilderId);
}

QStringList MsaBuilderFactory::supportedMsaBuilderIds() const
{
    using namespace constants::MsaBuilder;
    QStringList msaBuilderIds;
    msaBuilderIds << kClustalWBuilderId
                  << kKalignBuilderId;
    return msaBuilderIds;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
ClustalWMsaBuilder *MsaBuilderFactory::makeClustalWMsaBuilder() const
{
    ClustalWMsaBuilder *builder = new ClustalWMsaBuilder;
    QString programPath = fullProgramPath(constants::kClustalWRelativePath);
    builder->setProgram(programPath);
    return builder;
}

KalignMsaBuilder *MsaBuilderFactory::makeKalignMsaBuilder() const
{
    KalignMsaBuilder *builder = new KalignMsaBuilder;
    QString programPath = fullProgramPath(constants::kKalignRelativePath);
    builder->setProgram(programPath);
    return builder;
}

QString MsaBuilderFactory::fullProgramPath(const QString &relativeProgramPath) const
{
    return qApp->applicationDirPath() +
           QDir::separator() +
           relativeProgramPath;
}
