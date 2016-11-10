/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CLUSTALFORMATINSPECTOR_H
#define CLUSTALFORMATINSPECTOR_H

#include "AbstractDataFormatInspector.h"

class ClustalFormatInspector : public AbstractDataFormatInspector
{
public:
    // ------------------------------------------------------------------------------------------------
    // Reimplemented public functions from base class
    DataFormatType inspect(const QString &buffer) const;    //!< Performs cursory inspection of buffer and returns eClustalType if it appears to be a Clustal formatted string or eUnknownType otherwise
};

#endif // CLUSTALFORMATINSPECTOR_H
