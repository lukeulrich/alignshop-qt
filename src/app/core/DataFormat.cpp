/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DataFormat.h"
#include "Parsers/ISequenceParser.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param type [DataFormatType]
  * @param name [const QString &]
  * @param fileExtensions [const QStringList &]
  * @param sequenceParser [ISequenceParser *]
  */
DataFormat::DataFormat(DataFormatType type,
                       const QString &name,
                       const QStringList &fileExtensions,
                       ISequenceParser *sequenceParser) :
        type_(type), name_(name), sequenceParser_(sequenceParser)
{
    setFileExtensions(fileExtensions);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * @param other [const DataFormat &]
  * @returns bool
  */
bool DataFormat::operator==(const DataFormat &other) const
{
    return type_ == other.type_ &&
            name_ == other.name_ &&
            fileExtensions_ == other.fileExtensions_ &&
            sequenceParser_ == other.sequenceParser_;
}

/**
  * @param other [const DataFormat &]
  * @returns bool
  */
bool DataFormat::operator!=(const DataFormat &other) const
{
    return !operator==(other);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QStringList
  */
QStringList DataFormat::fileExtensions() const
{
    return fileExtensions_;
}

/**
  * @returns DataFormatType
  */
DataFormatType DataFormat::type() const
{
    return type_;
}

/**
  * Returns false if trimmed extension is empty or it is not present as determined by a case-insensitive search
  * within extensions_.
  *
  * @param extension [const QString &]
  * @returns bool
  */
bool DataFormat::hasFileExtension(const QString &extension) const
{
    return fileExtensions_.contains(extension.trimmed(), Qt::CaseInsensitive);
}

/**
  * @returns QString
  */
QString DataFormat::name() const
{
    return name_;
}

/**
  * If name_ is not empty, then put a space between it and the list of wildcard extension filters; otherwise,
  * simply return the wildcard filters.
  *
  * @returns QString
  */
QString DataFormat::nameFilter() const
{
    QString nameFilterString;

    if (fileExtensions_.count() == 0)
        return nameFilterString;

    if (!name_.isEmpty())
        nameFilterString = name_ + ' ';

    nameFilterString += "(*." + fileExtensions_.join(" *.") + ')';

    return nameFilterString;
}

/**
  * @returns ISequenceParser *
  */
ISequenceParser *DataFormat::parser() const
{
    return sequenceParser_;
}

/**
  * Ignores duplicate and empty extensions.
  *
  * @param fileExtensions [const QStringList &]
  */
void DataFormat::setFileExtensions(const QStringList &fileExtensions)
{
    fileExtensions_.clear();

    for (int i=0, z=fileExtensions.count(); i<z; ++i)
    {
        QString extension = fileExtensions.at(i).trimmed();
        if (!extension.isEmpty())
            fileExtensions_ << extension;
    }

    // Remove all duplicate strings
    fileExtensions_.removeDuplicates();
}

/**
  * @param format [FileFormat::Format]
  */
void DataFormat::setType(DataFormatType type)
{
    type_ = type;
}

/**
  * @param name [const QString &]
  */
void DataFormat::setName(const QString &name)
{
    name_ = name;
}

/**
  * @param parser [ISequenceParser *]
  */
void DataFormat::setParser(ISequenceParser *sequenceParser)
{
    sequenceParser_ = sequenceParser;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static methods
/**
  * Ignores all NULL DataFormat pointers.
  *
  * @param fileFormats [const QList<FileFormat> &]
  * @returns QStringList
  */
QStringList DataFormat::nameFilters(const QVector<DataFormat> &dataFormats)
{
    QStringList nameFilters;

    foreach (const DataFormat &f, dataFormats)
    {
        QString nameFilter = f.nameFilter();
        if (nameFilter.isEmpty())
            continue;

        nameFilters << nameFilter;
    }

    return nameFilters;
}
