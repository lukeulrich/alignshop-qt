/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "enums.h"
#include "global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class ISequenceParser;

/**
  * DataFormat encapsulates the specification and utility methods related to a particular format.
  *
  * Generally, a data format may be described as having:
  * o A unique integer identifying the type of file (DataFormatType enum)
  * o An arbitrary name
  * o A list of file extensions common to files containing such data
  *
  * Additionally, it is possible to associate an optional AbstractDataFormatInspector pointer which may be
  * requested by client classes for inspecting a buffer if it conforms to this data format specification.
  * DataFormat takes ownership of any supplied AbstractDataFormatInspector.
  *
  * All extensions are handled in a case-insensitive manner and only the suffix without the leading period
  * should be provided (e.g. 'faa', not '.faa').
  */
class DataFormat
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Constructs an instance of this object using format (default: UnknownFormat), name (default: empty), extensions (default: empty), and parser (default: NULL)
    DataFormat(DataFormatType format = eUnknownFormat,
               const QString &name = QString(),
               const QStringList &fileExtensions = QStringList(),
               ISequenceParser *sequenceParser = nullptr);

    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const DataFormat &other) const;
    bool operator!=(const DataFormat &other) const;

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QStringList fileExtensions() const;                                     //!< Returns the list of extensions
    DataFormatType type() const;                                            //!< Returns the specific enum Format for this instance
    bool hasFileExtension(const QString &extension) const;                  //!< Returns true if extension is present within the private list of extensions (case-insensitive) or false otherwise
    QString name() const;                                                   //!< Returns the name of this File Format
    QString nameFilter() const;                                             //!< Returns a QFileDialog compatible nameFilter if at least one extension is present; otherwise returns an empty string
    ISequenceParser *parser() const;                                        //!< Returns a pointer to an ISequenceParser instance

    void setFileExtensions(const QStringList &fileExtensions);              //!< Set the list of unique, non-empty extensions (must have at least one non-whitespace character) to extensions
    void setType(DataFormatType type);                                      //!< Set the format type
    void setName(const QString &name);                                      //!< Set the name to name
    void setParser(ISequenceParser *sequenceParser);                        //!< Set the sequence parser pointer to parser; does not take ownership

    // ------------------------------------------------------------------------------------------------
    // Static methods
    //! Return a string list of QFileDialog compatible nameFilters for those dataFormats that have both a non-empty name and at least one extension
    static QStringList nameFilters(const QVector<DataFormat> &dataFormats);

private:
    DataFormatType type_;                                     //!< Enumerated data format type
    QString name_;                                            //!< Arbitrary name for this file format
    QStringList fileExtensions_;                              //!< List of file extensions used to identify this data format
    ISequenceParser *sequenceParser_;                         //!< Pointer to parser instance
};

Q_DECLARE_TYPEINFO(DataFormat, Q_MOVABLE_TYPE);

#endif // DATAFORMAT_H
