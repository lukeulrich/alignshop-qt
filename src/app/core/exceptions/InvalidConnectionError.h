/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef INVALIDCONNECTIONERROR_H
#define INVALIDCONNECTIONERROR_H

#include "RuntimeError.h"

class InvalidConnectionError : public RuntimeError
{
public:
    InvalidConnectionError(const QString &message, const QString &connectionName);

    const QString connectionName() const;

protected:
    QString connectionName_;
};

inline
InvalidConnectionError::InvalidConnectionError(const QString &message, const QString &connectionName)
    : RuntimeError(message), connectionName_(connectionName)
{
}

inline
const QString InvalidConnectionError::connectionName() const
{
    return connectionName_;
}

#endif // INVALIDCONNECTIONEXCEPTION_H
