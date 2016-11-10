/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef RUNTIMEERROR_H
#define RUNTIMEERROR_H

#include "Exception.h"

/**
  * RuntimeError's require a QString during construction which enables dynamic messages to be reported
  * in conjunction with an exceptional circumstance or relevant error.
  *
  * The message argument passed in to the constructor is returned via the what() method.
  */
class RuntimeError : public Exception
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    RuntimeError(const QString &message);       //!< Constructs an exception message

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual const QString what() const;         //!< Returns the message supplied to the constructor

protected:
    QString message_;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param message [const QString &]
  */
inline
RuntimeError::RuntimeError(const QString &message) : Exception(), message_(message)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Not capable of throwing any exceptions.
  *
  * @returns const QString
  */
inline
const QString RuntimeError::what() const
{
    return message_;
}

#endif // RUNTIMEERROR_H
