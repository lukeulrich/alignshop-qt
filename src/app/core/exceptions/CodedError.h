/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CODEDERROR_H
#define CODEDERROR_H

#include "RuntimeError.h"

class CodedError : public RuntimeError
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    CodedError(int errorNumber, const QString &message);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int errorNumber() const;

protected:
    int errorNumber_;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
inline
CodedError::CodedError(int errorNumber, const QString &message)
    : RuntimeError(message), errorNumber_(errorNumber)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
inline
int CodedError::errorNumber() const
{
    return errorNumber_;
}


#endif // CODEDERROR_H
