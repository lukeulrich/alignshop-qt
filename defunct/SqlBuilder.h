/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SQLBUILDER_H
#define SQLBUILDER_H

#include <QtCore/QString>

/**
  * SqlBuilder provides a set of static methods for solely constructing well-formed SQL code.
  *
  * None of these methods check that tableNames and/or fields have the proper syntax and form. That
  * is the responsibility of the calling code.
  */
class SqlBuilder
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public static methods
    //! Builds and returns a parameterized SQL insert string for inserting fields into tableName; returns an empty QString if tableName or fields is empty
    static QString preparedInsertSql(const QString &tableName, const QStringList &fields);
    //! Builds and returns a parameterized SQL update string for updating fields within tableName that match conditions; returns an empty QString if tableName or fields is empty
    static QString preparedUpdateSql(const QString &tableName, const QStringList &fields, const QString &conditions);
};

#endif // SQLBUILDER_H
