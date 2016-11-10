/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "CrudSqlRecord.h"

#include <QtCore/QHash>
#include <QtCore/QStringBuilder>

#include <QtSql/QSqlError>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * SELECT's queryFields_ from dbTable.database() WHERE dbTable.primaryKeyField() = id and stores the results
  * locally. By default, queryFields_ contains all the fields. If id is not provided, id_ will be used in its
  * place. To illustrate this, the following commands are equivalent:
  *
  * CrudSqlRecord project(projectTable);
  * project.id_ = 50;
  * project.read()
  * // OR
  * project.read(50);
  *
  * The class member, id_, is set to id if id is provided regardless if it is not in the select list.
  * For example,
  * CrudSqlRecord project(projectTable);
  * project.read(25);   // project.id_ is now 25
  *
  * Conditions which do nothing and return -1:
  * o this is empty (no fields are selected to be returned)
  * o dbTable_ is empty
  * o Both id and id_ are empty
  *
  * If a database error occurs, -2 will be returned and more information may be obtained by calling dbTable().lastError()
  *
  * Only one record will be read. If abnormal database table and/or multiple records contain the same value for
  * the primaryKeyField, the specific row which will be returned is undefined.
  *
  * @param id [const QVariant &]; default QVariant()
  * @returns int
  */
int CrudSqlRecord::read(const QVariant &id)
{
    if (isEmpty() || dbTable_.isEmpty())
        return -1;

    // Update id_
    if (!id.isNull())
        id_ = id;
    else if (id_.isNull())
        return -1;

    // Build the query SQL; we add in the primaryKeyField() so that is always included in the output
    QString select = "SELECT " % dbTable_.primaryKeyField() % ", " % queryFields().join(", ") %
                     " FROM " % dbTable_.tableName() %
                     " WHERE " % dbTable_.primaryKeyField() % " = ? LIMIT 1";
    QSqlQuery query(dbTable_.database());

    // Conserve memory and perhaps performance as well
    query.setForwardOnly(true);
    if (!query.prepare(select))
        return -2;

    query.bindValue(0, id_);

    // Clear the class member variables
    clearValues();

    // Execute the select query
    if (query.exec())
    {
        if (query.next())
        {
            for (int i=0, z=count(); i<z; ++i)
                setValue(i, query.value(i+1));

            return 1;
        }

        // Query successful, zero rows found
        return 0;
    }

    // A database error occurred
    return -2;
}

/**
  * DELETE's the record FROM dbTable.tableName() WHERE dbTable.primaryKeyField() = id and clears any locally stored
  * data. If id is NULL, then id_ will be used in its place. Sets id_ to an invalid QVariant (which may also be tested
  * with isNull() because that will return true for an invalid QVariant).
  *
  * Conditions which do nothing and return -1:
  * o Both id and id_ are empty
  * o dbTable_ is empty
  *
  * Otherwise, returns -2 if there was a database error or the number of rows that were deleted (which may be more than
  * 1 if there are duplicate values for primaryKeyField())
  *
  * @param id [const QVariant &]; default QVariant()
  * @returns int
  */
int CrudSqlRecord::erase(const QVariant &id)
{
    clearValues();
    QVariant deleteId = id;
    if (deleteId.isNull())
        deleteId = id_;

    if (deleteId.isNull())
        return -1;

    id_.clear();

    if (dbTable_.isEmpty())
        return -1;


    QString sql = "DELETE FROM " % dbTable_.tableName() %
                  " WHERE " % dbTable_.primaryKeyField() % " = ?";
    QSqlQuery query(dbTable_.database());

    // Conserve memory and perhaps performance as well
    query.setForwardOnly(true);
    if (!query.prepare(sql))
        return -2;

    query.bindValue(0, deleteId);

    // Execute the delete query
    if (query.exec())
    {
        int rows_affected = query.numRowsAffected();
        if (rows_affected == -1)
            return 0;

        return rows_affected;
    }

    return -2;
}

/**
  * This function INSERT's queryFields_ if id_ is not defined or UPDATE's queryFields_ if id_ is defined.
  * If id_ is not defined, INSERT's queryFields_ INTO dbTable.tableName() and if successful, sets id_ to the value of
  * dbTable_.primaryKeyField() for this record.
  *
  * Because an INSERT is performed anytime id_ is not defined and this variable is independent of the actual fields
  * that will be inserted, it is possible to both save a record with a given primary key value or have the database
  * generate this identifier if the database supports this operation.
  *
  * For example, given the table:
  * create table projects (
  *     id integer primary key autoincrement,
  *     name text not null
  * );
  *
  * o To have the database automatically generate the value for the id field:
  *   CrudSqlReport record(DbTable("projects", <QSqlDatabase>));
  *   record.id_.clear();  // By default it is already cleared, but clear if it has been set by previous CRUD operations
  *   record.remove("id"); // To ensure that the field is not included in the insert statement, we remove it. Alternatively,
  *                        // you may clear this value for the same effect.
  *   record.setValue("name", "My project");
  *   record.save();
  *   int newId = record.id_.toInt();  // record.id_ is set to the last insert id *if automatically generated by the database*
  *
  * o To INSERT a record with a specific value for the primary key field:
  *   record.id_.clear();              // Make sure that the id_ field is empty
  *   record.setValue("id", 1000);
  *   record.save();
  *   record.id_.toInt() == 1000;
  *
  * NOTE: If no fields are present in this record, will perform: INSERT INTO dbTable_.tableName() default values;
  *
  * On the other hand, if id_ is defined, then UPDATE dbTable.tableName() SET's ... WHERE dbTable.primaryKeyField() = id_.
  * If the primary key is updated, the corresponding id_ should be changed to reflect its new value.
  *
  * record.id_ = 5;
  * record.setValue("id", 10);
  * record.save();              // record.id_ is now 10
  *
  * Returns the number of rows inserted, -1 if configuration/input error, or -2 if a database error occurred.
  *
  * @returns int
  */
int CrudSqlRecord::save()
{
    if (dbTable_.isEmpty())
        return -1;

    QSqlQuery query(dbTable().database());
    query.setForwardOnly(true);

    // INSERT
    if (id_.isNull())
    {
        QString sql = "INSERT INTO " % dbTable_.tableName();
        QStringList fieldNames = queryFields();
        int nFields = fieldNames.count();
        if (nFields)
            sql += "(" % fieldNames.join(", ") % ") VALUES (?" % QString(", ?").repeated(nFields - 1) % ")";
        else
            sql += " default values";

        if (!query.prepare(sql))
        {
            error(query.lastError().text());
            return -2;
        }

        for (int i=0; i< nFields; ++i)
            query.bindValue(i, value(i));

        if (query.exec())
        {
            // Did the insert provide a value for the primaryKeyField?
            if (value(dbTable_.primaryKeyField()).isNull() == false)
                id_ = value(dbTable_.primaryKeyField());
            // Otherwise, if it is auto-generated, retrieve this value from the database
            else if (dbTable_.field(dbTable_.primaryKeyField()).isAutoValue())
                id_ = query.lastInsertId();

            return 1;
        }
        else
        {
            error(query.lastError().text());
            return -2;
        }
    }
    // UPDATE
    else
    {
        QStringList fieldNames = queryFields();
        int nFields = fieldNames.count();
        if (nFields == 0)
            return -1;

        QString sql = "UPDATE " % dbTable_.tableName() %
                      " SET " % fieldNames.join(" = ?, ") % " = ?"
                      " WHERE " % dbTable_.primaryKeyField() % " = ?";

        if (!query.prepare(sql))
        {
            error(query.lastError().text());
            return -2;
        }

        for (int i=0; i< nFields; ++i)
            query.bindValue(i, value(i));

        query.bindValue(nFields, id_);

        if (query.exec())
        {
            // Did the update provide a value for the primaryKeyField?
            if (value(dbTable_.primaryKeyField()).isNull() == false)
                id_ = value(dbTable_.primaryKeyField());

            return query.numRowsAffected();
        }
        else
        {
            error(query.lastError().text());
            return -2;
        }
    }

    // Catch-all that should not happen
    Q_ASSERT_X(0, "CrudSqlRecord::save", "Invalid location");
    return -1;
}
