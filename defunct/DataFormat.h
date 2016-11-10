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
#include <QtCore/QScopedPointer>

#include "global.h"
#include "AbstractDataFormatInspector.h"
#include "SimpleSeqParser.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AbstractDataFormatInspector;

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
    //! Constructs an instance of this object using format (default: UnknownFormat), name (default: empty), extensions (default: empty), and inspector (default: NULL)
    DataFormat(DataFormatType format = eUnknownFormatType, const QString &name = QString(), const QStringList &fileExtensions = QStringList(), AbstractDataFormatInspector *inspector = 0, SimpleSeqParser *parser = 0);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QStringList fileExtensions() const;                                     //!< Returns the list of extensions
    DataFormatType type() const;                                            //!< Returns the specific enum Format for this instance
    bool hasFileExtension(const QString &extension) const;                  //!< Returns true if extension is present within the private list of extensions (case-insensitive) or false otherwise
    AbstractDataFormatInspector *inspector() const;                         //!< Returns a pointer to the AbstractDataFormatInspector instance
    QString name() const;                                                   //!< Returns the name of this File Format
    QString nameFilter() const;                                             //!< Returns a QFileDialog compatible nameFilter if at least one extension is present; otherwise returns an empty string
    SimpleSeqParser *parser() const;                                        //!< Returns a pointer to the SimpleSeqParser instance

    void setFileExtensions(const QStringList &fileExtensions);              //!< Set the list of unique, non-empty extensions (must have at least one non-whitespace character) to extensions
    void setType(DataFormatType type);                                      //!< Set the Format to format
    void setInspector(AbstractDataFormatInspector *inspector);              //!< Set the inspector pointer to inspector; takes ownership of this pointer
    void setName(const QString &name);                                      //!< Set the name to name
    void setParser(SimpleSeqParser *parser);                                //!< Set the parser pointer to parser; takes ownership of this pointer

    // ------------------------------------------------------------------------------------------------
    // Static methods
    //! Return a list of QFileDialog compatible nameFilters for all FileFormat items in fileFormats that have both a non-empty name and at least one extension
    static QStringList nameFilters(const QList<DataFormat *> &dataFormats);

private:
    Q_DISABLE_COPY(DataFormat)

    DataFormatType type_;                                     //!< Enumerated data format type
    QString name_;                                            //!< Arbitrary name for this file format
    QStringList fileExtensions_;                              //!< List of file extensions used to identify this data format
    QScopedPointer<AbstractDataFormatInspector> inspector_;   //!< Scoped pointer to inspector instance
    QScopedPointer<SimpleSeqParser> parser_;                  //!< Scoped pointer to parser instance
};

#endif // DATAFORMAT_H
