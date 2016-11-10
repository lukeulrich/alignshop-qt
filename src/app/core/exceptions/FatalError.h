/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef FATALERROR_H
#define FATALERROR_H

#include "CodedError.h"

class FatalError : public CodedError
{
public:
    enum ErrorCode {
        eReopenDatabaseCode = 1     //!< Unable to re-open database file (e.g. during saveAs)
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor
    FatalError(int errorNumber, const QString &message);
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
inline
FatalError::FatalError(int errorNumber, const QString &message)
    : CodedError(errorNumber, message)
{
}


#endif // FATALERROR_H
