/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DataFormatDetector.h"

#include <QtCore/QFileInfo>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Initialize the private static members
QScopedPointer<DataFormat> DataFormatDetector::unknownFormat_(new DataFormat(eUnknownFormatType));


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * Trivial constructor
  */
DataFormatDetector::DataFormatDetector()
{
}

/**
  * De-allocate any heap-allocated memory
  */
DataFormatDetector::~DataFormatDetector()
{
    qDeleteAll(dataFormats_);
    dataFormats_.clear();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QList<DataFormat *>
  */
QList<DataFormat *> DataFormatDetector::dataFormats() const
{
    return dataFormats_;
}

/**
  * Iterates through all associated data formats and returns a pointer to the first DataFormat which
  * contains fileExtension. If none of the user-supplied data formats contain fileExtension, then
  * unknownFormat_ is returned.
  *
  * @param extension [const QString &]
  * @returns DataFormat *
  */
DataFormat *DataFormatDetector::formatFromFileExtension(const QString &fileExtension) const
{
    foreach (DataFormat *dataFormat, dataFormats_)
    {
        Q_ASSERT_X(dataFormat, "DataFormatDetector::formatFromFileExtension", "null dataFormat * not allowed here");

        if (dataFormat &&
            dataFormat->hasFileExtension(fileExtension))
        {
            return dataFormat;
        }
    }

    return unknownFormat_.data();
}

/**
  * Simply calls formatFromString with a chunk of header data extracted from file. If formatFromString
  * returns unknownFormat_, then attempt to determine the data format from its extension.
  *
  * Returns unkownFormat_ if both of the above techniques do not match a specific DataFormat.
  *
  * Reads 100k of header data to determine the file type
  *
  * @param file [const QFile &]
  * @returns DataFormat *
  */
DataFormat *DataFormatDetector::formatFromFile(QFile &file) const
{
    Q_ASSERT_X(file.exists(), "DataFormatDetector::formatFromFile", QString("File '%1' does not exist").arg(file.fileName()).toAscii());

    if (file.exists())
    {
        if (file.openMode() == QIODevice::NotOpen &&
            !file.open(QIODevice::ReadOnly))
        {
            return unknownFormat_.data();
        }

        // Should have an open file at this point, read a chunk of data from the beginning of the file
        file.seek(0);

        // Read the first 100kb
        DataFormat *dataFormat = formatFromString(file.read(1024 * 100));
        file.close();

        if (dataFormat->type() != eUnknownFormatType)
            return dataFormat;
        else
        {
            // Attempt from the file extension
            QFileInfo fi(file);
            return formatFromFileExtension(fi.suffix());
        }
    }

    return unknownFormat_.data();
}

/**
  * Iteratres through all associated data formats and returns a DataFormat pointer to the first one
  * that has a valid AbstractDataFormatInspector which does not return eUnknownType. If no valid
  * inspectors are present or all defined inspectors return eUnknownType, returns unknownFormat_.
  *
  * @param string [const QString &]
  * @returns DataFormat *
  */
DataFormat *DataFormatDetector::formatFromString(const QString &string) const
{
    foreach (DataFormat *dataFormat, dataFormats_)
    {
        Q_ASSERT_X(dataFormat, "DataFormatDetector::formatFromString", "null dataFormat * not allowed here");

        if (dataFormat &&
            dataFormat->inspector() &&
            dataFormat->inspector()->inspect(string) != eUnknownFormatType)
        {
            return dataFormat;
        }
    }

    return unknownFormat_.data();
}

/**
  * First frees any DataFormats that have been previously assigned, and then assigns dataFormats to
  * dataFormats_. Thus, it is vital that if the calling scope takes precaution that it does not have
  * any dangling pointers remaining. For instance:
  *
  * QList<DataFormat *> formats;
  * formats << new DataFormat(eFastaType);
  * setDataFormats(formats);
  *
  * // Do some work
  *
  * setDataFormats(QList<DatFormat *>());
  * // All contents of formats now contains dangling pointers!!
  * // Any access of formats will result in a segfault and crash the program!!
  *
  * @param dataFormats [const QList<DataFormat *> &]
  */
void DataFormatDetector::setDataFormats(const QList<DataFormat *> &dataFormats)
{
    qDeleteAll(dataFormats_);
    dataFormats_.clear();

    dataFormats_ = dataFormats;
}

/**
  * In some cases, it is desirable to have a pointer to the unknown data format instance that is
  * common to all instances of this class. This function returns a pointer to that instance.
  *
  * Important! Do not delete the pointer returned by this function. It is automatically de-allocated
  * when the program terminates via a ScopedPointer.
  *
  * @returns DataFormat *
  */
DataFormat *DataFormatDetector::unknownFormat() const
{
    return unknownFormat_.data();
}
