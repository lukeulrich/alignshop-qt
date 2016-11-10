/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DATABASEERROR_H
#define DATABASEERROR_H

#include <QtSql/QSqlError>

#include "RuntimeError.h"

class DatabaseError : public RuntimeError
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    DatabaseError(const QString &message = QString(), const QSqlError &sqlError = QSqlError(), const QString &sql = QString());

    // ------------------------------------------------------------------------------------------------
    // Public methods
    const QString sql() const;
    const QSqlError &sqlError() const;

protected:
    QSqlError sqlError_;
    QString sql_;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param message [const QString &]
  * @param errorType [QSqlError::ErrorType]
  */
inline
DatabaseError::DatabaseError(const QString &message, const QSqlError &sqlError, const QString &sql)
    : RuntimeError(message), sqlError_(sqlError), sql_(sql)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns const QSqlError &
  */
inline
const QSqlError &DatabaseError::sqlError() const
{
    return sqlError_;
}

/**
  * @returns const QString
  */
inline
const QString DatabaseError::sql() const
{
    return sql_;
}

#endif // DATABASEERROR_H
