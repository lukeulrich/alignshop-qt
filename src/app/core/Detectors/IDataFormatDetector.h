/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IDATAFORMATDETECTOR_H
#define IDATAFORMATDETECTOR_H

class QString;
class QFile;
class DataFormat;

class IDataFormatDetector
{
public:
    virtual ~IDataFormatDetector() {}

    //! Determine the format from extension and return a pointer to the corresponding DataFormat
    virtual DataFormat formatFromFileExtension(const QString &fileExtension) const = 0;
    virtual DataFormat formatFromFile(QFile &file) const = 0;             //!< Returns a pointer to the corresponding DataFormat of file based on its contents
    virtual DataFormat formatFromString(const QString &string) const = 0; //!< Determine the format by inspecting the contents of string and return FileFormat
};

#endif // IDATAFORMATDETECTOR_H
