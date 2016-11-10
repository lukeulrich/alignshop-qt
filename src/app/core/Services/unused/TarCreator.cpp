/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>

#include "TarCreator.h"
#include "../util/TarUtil.h"
#include "../PODs/TarHeaderPod.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param target [QIODevice *]
  * @param parent [QObject *]
  */
TarCreator::TarCreator(QIODevice *target, QObject *parent)
    : Pipe(target, parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param fileName [const QString &]
  * @returns bool
  */
bool TarCreator::archive(const QString &fileName)
{
    QFile file(fileName);
    if (file.exists())
    {
        setErrorString(QString("File does not exist: %1").arg(fileName));
        return false;
    }

    // Write the header
    TarUtil tarUtil;
    TarHeaderPod pod;
    try
    {
        pod = tarUtil.header(fileName);
    }
    catch (QString &error)
    {
        setErrorString(error);
        return false;
    }

    TarHeaderRawPod rawPod = tarUtil.toRawPod(pod);
    if (!writeDataToTarget(rawPod.rawData_, kTarHeaderSize))
        return false;

    // Dump the data
    if (!file.open(QIODevice::ReadOnly))
    {
        setErrorString(file.errorString());
        return false;
    }

    char buffer[16384];
    while (!file.atEnd())
    {
        qint64 bytesRead = file.read(buffer, 16384);
        if (bytesRead == -1)
        {
            setErrorString(file.errorString());
            return false;
        }

        if (!writeDataToTarget(buffer, bytesRead))
            return false;
    }
    file.close();

    return true;
}

/**
  */
void TarCreator::writeEofMarker()
{
    static char eofMarker[1024] = {0};

    writeDataToTarget(eofMarker, 1024);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param data [const char *]
  * @param len [qint64]
  * @returns qint64
  */
qint64 TarCreator::writeData(const char * /* data */, qint64 /* len */)
{
    setErrorString("Writing not allowed for the TarCreator class.");
    return -1;
}
