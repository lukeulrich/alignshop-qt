/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFileInfo>

#include "DataFormatDetector.h"
#include "../Parsers/ISequenceParser.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QVector<DataFormat>
  */
QVector<DataFormat> DataFormatDetector::dataFormats() const
{
    return dataFormats_;
}

/**
  * Iterates through all associated data formats and returns the first DataFormat which contains fileExtension. If none
  * of the user-supplied data formats contain fileExtension, then a default constructed DataFormat is returned.
  *
  * @param fileExtension [const QString &]
  * @returns DataFormat
  */
DataFormat DataFormatDetector::formatFromFileExtension(const QString &fileExtension) const
{
    foreach (const DataFormat &dataFormat, dataFormats_)
        if (dataFormat.hasFileExtension(fileExtension))
            return dataFormat;

    return DataFormat();
}

/**
  * Simply calls formatFromString with a chunk of header data extracted from file. If formatFromString returns an
  * unrecognized format, then attempt to determine the data format from its extension.
  *
  * Returns a default constructed DataFormat if both of the above techniques do not match a specific DataFormat.
  *
  * Reads 10k of header data to determine the file type.
  *
  * @param file [QFile &]
  * @returns DataFormat
  */
DataFormat DataFormatDetector::formatFromFile(QFile &file) const
{
    Q_ASSERT_X(file.exists(), "DataFormatDetector::formatFromFile", QString("File '%1' does not exist").arg(file.fileName()).toAscii());

    if (file.openMode() == QIODevice::NotOpen &&
            !file.open(QIODevice::ReadOnly))
    {
        return DataFormat();
    }

    // Should have an open file at this point, read a chunk of data from the beginning of the file
    file.seek(0);

    // Read the first 100kb
    DataFormat dataFormat = formatFromString(file.read(1024 * 10));
    file.close();

    if (dataFormat.type() != eUnknownFormat)
        return dataFormat;

    // Attempt from the file extension
    QFileInfo fi(file);
    return formatFromFileExtension(fi.suffix());
}

/**
  * Iteratres through all associated data formats and returns the DataFormat for the first one that has a valid parser
  * that has returns true for compatible strings. If no valid parsers are preseent or all parsers return false for
  * the compatibility test, return a default constructed DataFormat.
  *
  * @param string [const QString &]
  * @returns DataFormat
  * @see ISequenceParser::isCompatibleString()
  */
DataFormat DataFormatDetector::formatFromString(const QString &string) const
{
    foreach (const DataFormat &dataFormat, dataFormats_)
    {
        if (dataFormat.parser() != nullptr &&
            dataFormat.parser()->isCompatibleString(string))
        {
            return dataFormat;
        }
    }

    return DataFormat();
}

/**
  * @param dataFormats [const QVector<DataFormat> &]
  */
void DataFormatDetector::setDataFormats(const QVector<DataFormat> &dataFormats)
{
    dataFormats_ = dataFormats;
}
