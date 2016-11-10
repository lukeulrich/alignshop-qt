/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IDBSOURCE_H
#define IDBSOURCE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class IDbSource
{
public:
    virtual QSqlDatabase database() const = 0;
    virtual QSqlQuery getPreparedQuery(const QString &name, const QString &sql, bool forwardOnly = true) = 0;
    virtual void clearPreparedQueries() = 0;
};

#endif // IDBSOURCE_H
