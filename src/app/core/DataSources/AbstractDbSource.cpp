/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QString>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "AbstractDbSource.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param name [const QString &]
  * @param sql [const QString &]
  * @param forwardOnly [bool]
  * @returns QSqlQuery
  */
QSqlQuery AbstractDbSource::getPreparedQuery(const QString &name, const QString &sql, bool forwardOnly)
{
    if (!preparedQueries_.contains(name))
    {
        QSqlQuery query = QSqlQuery(database());
        if (!query.prepare(sql))
        {
            qDebug() << Q_FUNC_INFO << query.lastError().text();
            throw 0;
        }

        query.setForwardOnly(forwardOnly);
        preparedQueries_.insert(name, query);
    }

    return preparedQueries_.value(name);
}

/**
  */
void AbstractDbSource::clearPreparedQueries()
{
    preparedQueries_.clear();
}
