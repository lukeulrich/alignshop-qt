/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QtCore/QString>

/**
  * Exception provides a universal base class for all exception objects.
  *
  * Exception is virtually identical to the std::exception class; however, it utilizes QString in place of
  * std::string and/or const char *.
  */
class Exception
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual const QString what() const;     //!< Returns a constant generic QString
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Not capable of throwing any exceptions.
  *
  * @returns const QString
  */
inline
const QString Exception::what() const
{
    return "An unknown exception has occurred";
}

#endif // EXCEPTION_H
