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
#include <QtCore/QVector>

#include "IDataFormatDetector.h"
#include "../DataFormat.h"

/**
  * DataFormatDetector automatically determines the data format from either an extension, an arbitrary string, or a file
  * object.
  *
  * The methods defined here attempt to examine various forms of user input and deduce its corresponding
  * format. If no format can be determined, it returns a default constructed DataFormat.
  */
class DataFormatDetector : public IDataFormatDetector
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    QVector<DataFormat> dataFormats() const;                            //!< Return the list of associated data formats used for detection purposes
    //! Determine the format from extension and return a pointer to the corresponding DataFormat
    virtual DataFormat formatFromFileExtension(const QString &fileExtension) const;
    virtual DataFormat formatFromFile(QFile &file) const;               //!< Returns a pointer to the corresponding DataFormat of file based on its contents
    virtual DataFormat formatFromString(const QString &string) const;   //!< Determine the format by inspecting the contents of string and return FileFormat
    void setDataFormats(const QVector<DataFormat> &dataFormats);        //!< Set the list of DataFormats to use when detecting the DataFormat; takes ownership of QList<DataFormat *>

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected members
    QVector<DataFormat> dataFormats_;                                   //!< User-supplied list of data formats for detection purposes
};

#endif // DATAFORMATDETECTOR_H
