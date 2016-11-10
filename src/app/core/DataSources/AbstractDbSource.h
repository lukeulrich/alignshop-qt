/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTDBSOURCE_H
#define ABSTRACTDBSOURCE_H

#include <QtCore/QHash>

#include "IDbSource.h"

class AbstractDbSource : public IDbSource
{
public:
    QSqlQuery getPreparedQuery(const QString &name, const QString &sql, bool forwardOnly = true);
    void clearPreparedQueries();

private:
    QHash<QString, QSqlQuery> preparedQueries_;
};

#endif // ABSTRACTDBSOURCE_H
