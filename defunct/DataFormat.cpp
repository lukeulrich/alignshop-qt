/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DataFormat.h"

#include <QtCore/QStringBuilder>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param type[DataFormatType]
  * @param name [const QString &]
  * @param extensions [const QStringList &]
  * @param inspector [AbstractDataFormatInspector *]
  */
DataFormat::DataFormat(DataFormatType type, const QString &name, const QStringList &fileExtensions, AbstractDataFormatInspector *inspector, SimpleSeqParser *parser) :
        type_(type), name_(name), inspector_(inspector), parser_(parser)
{
    setFileExtensions(fileExtensions);
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
  * @returns AbstractDataFormatInspector *
  */
AbstractDataFormatInspector *DataFormat::inspector() const
{
    return inspector_.data();
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
        nameFilterString = name_ % ' ';

    nameFilterString += "(*." % fileExtensions_.join(" *.") % ')';

    return nameFilterString;
}

/**
  * @returns SimpleSeqParser *
  */
SimpleSeqParser *DataFormat::parser() const
{
    return parser_.data();
}

/**
  * Ignores duplicate and empty extensions.
  *
  * @param extensions [const QStringList &]
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
  * @param inspector [AbstractDataFormatInspector *]
  */
void DataFormat::setInspector(AbstractDataFormatInspector *inspector)
{
    inspector_.reset(inspector);
}

/**
  * @param name [const QString &]
  */
void DataFormat::setName(const QString &name)
{
    name_ = name;
}

/**
  * @param parser [SimpleSeqParser *]
  */
void DataFormat::setParser(SimpleSeqParser *parser)
{
    parser_.reset(parser);
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
QStringList DataFormat::nameFilters(const QList<DataFormat *> &dataFormats)
{
    QStringList nameFilters;

    foreach (DataFormat *f, dataFormats)
    {
        if (!f)
            continue;

        QString nameFilter = f->nameFilter();
        if (nameFilter.isEmpty())
            continue;

        nameFilters << nameFilter;
    }

    return nameFilters;
}
