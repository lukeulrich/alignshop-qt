/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef GENERROR_H
#define GENERROR_H

#include <QtCore/QString>

/**
  * General error class for simply reporting any error message in a non-specific manner.
  *
  * GenError contains one private member, message_, which may be specified upon construction and fetched for display
  * purposes via the what() method. This is a bare-bones, generic mechanism for handling and reporting error messages
  * during program execution.
  */
class GenError
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    GenError(QString message = QString());      //!< Construct a GenError with the string message
    GenError(const char *message);
    GenError(const GenError &other);            //!< Construct a copy of other

    // ------------------------------------------------------------------------------------------------
    // Operators
//    virtual GenError &operator=(const char *message);

//    virtual GenError &operator=(const QString &message);     //!< Set the message to message

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setMessage(QString message);           //!< Set the message
    virtual const QString &message() const;     //!< Return the error message

protected:
    QString message_;                           //!< Relevant error message
};


// -----------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param message [QString]
  */
inline
GenError::GenError(QString message) : message_(message)
{
}

/**
  * @param message [const char *]
  */
inline
GenError::GenError(const char *message) : message_(message)
{
}

/**
  * @param other [const GenError &]
  */
inline
GenError::GenError(const GenError &other)
{
    message_ = other.message_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @return QString
  */
inline
const QString &GenError::message() const
{
    return message_;
}

/**
  * @param message [QString]
  */
inline
void GenError::setMessage(QString message)
{
    message_ = message;
}

#endif // GENERROR_H
