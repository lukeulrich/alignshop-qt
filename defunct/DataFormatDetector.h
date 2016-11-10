/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DATAFORMATDETECTOR_H
#define DATAFORMATDETECTOR_H

#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

#include "DataFormat.h"
#include "global.h"

/**
  * DataFormatDetector provides both an abstract and concrete interface for automatically determining the data
  * format from either an extension, an arbitrary string, or a file object.
  *
  * The methods defined here attempt to examine various forms of user input and deduce its corresponding
  * format. If no format can be determined, it returns a pointer to a static initialized unknown type. Thus, it
  * is never possible to obtain a null pointer from the formatFrom* functions.
  *
  * >>> IMPORTANT: DO NOT DELETE THE POINTERS RETURNED BY THESE FUNCTIONS!!! <<<
  *
  * Takes ownership of the list of data formats supplied to this class.
  */
class DataFormatDetector
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructors
    DataFormatDetector();                                              //!< Trivial constructor
    virtual ~DataFormatDetector();                                     //!< Free all user-allocated data formats

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QList<DataFormat *> dataFormats() const;                           //!< Return the list of associated data formats used for detection purposes
    //! Determine the format from extension and return a pointer to the corresponding DataFormat
    virtual DataFormat *formatFromFileExtension(const QString &fileExtension) const;
    virtual DataFormat *formatFromFile(QFile &file) const;             //!< Returns a pointer to the corresponding DataFormat of file based on its contents
    virtual DataFormat *formatFromString(const QString &string) const; //!< Determine the format by inspecting the contents of string and return FileFormat
    void setDataFormats(const QList<DataFormat *> &dataFormats);       //!< Set the list of DataFormats to use when detecting the DataFormat; takes ownership of QList<DataFormat *>
    DataFormat *unknownFormat() const;                                 //!< Returns the static instance of the unknonw format

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected members
    QList<DataFormat *> dataFormats_;                                  //!< User-supplied list of data formats for detection purposes
    //! By making this a QScopedPointer, it should be automatically freed upon program termination
    static QScopedPointer<DataFormat> unknownFormat_;

private:
    Q_DISABLE_COPY(DataFormatDetector)
};

#endif // DATAFORMATDETECTOR_H
