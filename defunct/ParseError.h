/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef PARSEERROR_H
#define PARSEERROR_H

#include "GenError.h"

/**
  * A specialized class for catching errors during parsing.
  *
  * In addition to a simple error message, also contains the line number, column, and line that error occurred.
  */
class ParseError : public GenError
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Construct a ParseError with the string message for column and lineNumber that was found on line
    ParseError(QString message = QString(), const QString &line = QString(), int lineNumber = -1, int columnNumber = -1);
    ParseError(const ParseError &other);            //!< Construct a copy of other

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int columnNumber() const;                       //!< Return the column number where the error occurred
    int lineNumber() const;                         //!< Return the line number where the error occurred
    const QString &line() const;                    //!< Return the line where the error occurred

    //! Set all the parameters of the ParseError
    void set(QString message, const QString &line = QString(), int lineNumber = -1, int columnNumber = -1);

protected:
    QString line_;              //!< Line where error occurred
    int lineNumber_;            //!< Line number where error occurred
    int columnNumber_;          //!< Column number where error occurred
};

// -----------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param message [QString]
  * @param column [int]
  * @param lineNumber [int]
  * @param line [const QString &]
  */
inline
ParseError::ParseError(QString message, const QString &line, int lineNumber, int columnNumber) :
    GenError(message), line_(line), lineNumber_(lineNumber), columnNumber_(columnNumber)
{
}

/**
  * @param other [const ParseError &]
  */
inline
ParseError::ParseError(const ParseError &other) :
    GenError(other.message_), line_(other.line_), lineNumber_(other.lineNumber_), columnNumber_(other.columnNumber_)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @return int
  */
inline
int ParseError::columnNumber() const
{
    return columnNumber_;
}

/**
  * @return int
  */
inline
int ParseError::lineNumber() const
{
    return lineNumber_;
}

/**
  * @return const QString &
  */
inline
const QString &ParseError::line() const
{
    return line_;
}

/**
  * Set all the private members defining the exact error at hand
  *
  * @param message [QString]
  * @param columnNumber [int]
  * @param lineNumber [int]
  * @param
  */
inline
void ParseError::set(QString message, const QString &line , int lineNumber, int columnNumber)
{
    message_ = message;
    line_ = line;
    lineNumber_ = lineNumber;
    columnNumber_ = columnNumber;
}

#endif // PARSEERROR_H
