/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Tar.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
Tar::Tar(QObject *parent)
	: QObject()
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param device [QIODevice *]
  * @param directory [const QDir &]
  */
void Tar::extract(QIODevice *device, const QDir &directory)
{
}

/**
  * @param device [QIODevice *]
  * @param file [const QString &]
  */
void Tar::archive(QIODevice *device, const QString &file)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param device [QIODevice *]
  * @returns QByteArray
  */
QByteArray Tar::readTarHeader(QIODevice *device) const
{
    return QByteArray();
}

/**
  * @param file [const QString &]
  * @returns QByteArray
  */
QByteArray Tar::tarHeader(const QString &file) const
{
    return QByteArray();
}
