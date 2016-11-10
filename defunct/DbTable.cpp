/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DbTable.h"

#include <QSqlField>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * In essence, does the same thing as the constructor with one minor difference: if database is not valid or open,
  * then the stored database_ is used for reloading the table structure and database_ is not updated.
  *
  * @param tableName [const QString &]
  * @param database [const QSqlDatabase &]
  * @returns bool
  */
bool DbTable::setTable(const QString &tableName, const QSqlDatabase &database)
{
    if (tableName.isEmpty() ||
        (database.isValid() && !database.isOpen()) ||
        (!database.isValid() && !DbTable::database().isValid()))
    {
        return false;
    }

    QSqlRecord tableSpec;
    if (database.isOpen())
    {
        tableSpec = database.record(tableName);
        if (tableSpec.isEmpty())
            return false;

        databaseName_ = database.connectionName();
    }
    else    // Utilize the existing QSqlDatabase instance
    {
        tableSpec = DbTable::database().record(tableName);
        if (tableSpec.isEmpty())
            return false;
    }

    clear();
    for (int i=0, z=tableSpec.count(); i<z; ++i)
        append(tableSpec.field(i));

    tableName_ = tableName;
    _initializePrimaryKeyField();

    return true;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Checks for a primary key index on tableName_. If present and it contains only one field, set this field to the primaryKeyField_.
  * Otherwise, if it contains a field labeled, id, use that. Finally, if no identifying field has been found, simply use the first
  * field present.
  */
void DbTable::_initializePrimaryKeyField()
{
    Q_ASSERT_X(database().isValid(), "DbTable::_initializePrimaryKeyField", "database() is not valid");
    Q_ASSERT_X(database().isOpen(), "DbTable::_initializePrimaryKeyField", "database() is not open");
    Q_ASSERT_X(tableName_.isEmpty() == false, "DbTable::_initializePrimaryKeyField", "tableName_ is empty");
    Q_ASSERT_X(isEmpty() == false, "DbTable::_initializePrimaryKeyField", "No fields defined");

    // Determine primary key field
    QSqlIndex pk = database().primaryIndex(tableName_);
    if (pk.count() == 1)
        primaryKeyField_ = pk.fieldName(0);
    else if (contains("id"))
        primaryKeyField_ = "id";
    else
        primaryKeyField_ = fieldName(0);
}
