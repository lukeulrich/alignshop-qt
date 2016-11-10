/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef FASTAFORMATINSPECTOR_H
#define FASTAFORMATINSPECTOR_H

#include "AbstractDataFormatInspector.h"

class FastaFormatInspector : public AbstractDataFormatInspector
{
public:
    // ------------------------------------------------------------------------------------------------
    // Reimplemented public functions from base class
    DataFormatType inspect(const QString &buffer) const;    //!< Performs cursory inspection of buffer and returns eFastaType if it appears to be Fasta formatted or eUnknownType otherwise
};

#endif // FASTAFORMATINSPECTOR_H
