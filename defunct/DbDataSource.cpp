/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DbDataSource.h"

#include <QtCore/QStringBuilder>

#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include "exceptions/DatabaseError.h"
#include "exceptions/InvalidConnectionError.h"

#include <QtCore/QDebug>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * By default the connectionName is empty and uninitialized; however, if the user provides a non-empty
  * connectionName, then the connection will be validated by the setConnectionName method, which will
  * throw an InvalidConnectionError if it is invalid.
  *
  * @param connectionName [const QString &]
  */
DbDataSource::DbDataSource(const QString &connectionName)
{
    if (!connectionName.isEmpty())
        setConnectionName(connectionName);
}

/**
  */
DbDataSource::~DbDataSource()
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param tableName [const QString &]
  *
  * @throws InvalidConnectionError, DatabaseError
  */
void DbDataSource::checkTable(const QString &tableName) const
{
    if (!databaseReady())
        throw InvalidConnectionError(constants::kMessageInvalidConnection, connectionName_);

    if (tableName.isEmpty())
        throw DatabaseError(constants::kMessageErrorMissingTableName);

    if (database().record(tableName).isEmpty())
        throw DatabaseError(QString("Table %1 does not exist").arg(tableName));
}

/**
  * @returns QString
  */
QString DbDataSource::connectionName() const throw()
{
    return connectionName_;
}

/**
  * @returns QSqlDatabase
  */
QSqlDatabase DbDataSource::database() const throw()
{
    return QSqlDatabase::database(connectionName_);
}

/**
  * @returns bool
  */
bool DbDataSource::databaseReady() const throw()
{
    return database().isValid() && database().isOpen();
}

/**
  * Zero is returned if id is invalid.
  *
  * @param tableName [const QString &]
  * @param id [int]
  * @param conditions [const QString &]
  * @returns int
  *
  * @throws InvalidConnectionError, DatabaseError
  */
int DbDataSource::erase(const QString &tableName, QVariant id, const QString &conditions) const
{
    checkTable(tableName);

    if (!id.isValid())
        return 0;

    // Build the erase query
    QString erase_sql = "DELETE FROM " % tableName % " WHERE id = ?";
    if (!conditions.isEmpty())
        erase_sql += " AND " % conditions;

    // Prepare the query
    QSqlQuery erase_query(database());
    if (!erase_query.prepare(erase_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, erase_query.lastError(), erase_sql);

    // Bind the value
    erase_query.bindValue(0, id);

    // Execute
    if (!erase_query.exec())
        throw DatabaseError(constants::kMessageErrorExecutingQuery, erase_query.lastError(), erase_sql);

    return erase_query.numRowsAffected();
}

/**
  * If fields is empty, then all fields in the tableName are utilized. Returns false if dataRow.id_ is
  * valid.
  *
  * On success, dataRow.id_ is set to the last insert id if an id was not supplied by the user or it is null.
  * If duplicate field names are supplied, then the final insert result depends on the database system.
  *
  * If a field is specified but does not exist in the target database table, the query will fail.
  *
  * @param dataRow [DataRow &]
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @returns bool
  * @see insert(const QList<DataRow> &dataRows, const QString &tableName, const QStringList &fields)
  *
  * @throws InvalidConnectionError, DatabaseError
  */
bool DbDataSource::insert(DataRow &dataRow, const QString &tableName, const QStringList &fields) const
{
    checkTable(tableName);

    // Do not permit inserting records that have member id_
    if (dataRow.id_.isValid())
        return false;

    // Build the insert SQL
    QStringList insert_fields = (fields.isEmpty()) ? getTableFields(tableName) : fields;
    QString insert_sql = buildInsertSql(tableName, insert_fields);

    // It is impossible ofr insert_sql to be empty: 1) tableName and database exist by virtue of the checkTable function
    // (which would throw an exception if either of these conditions were false), 2) it's impossible to have a table
    // with no fields, 3) therefore if fields is empty, at least one field will be returned by getTableFields
    // Therefore, because buildInsertSql only returns an empty string if no fields are provided, it will return a non-empty
    // string.
    Q_ASSERT_X(!insert_sql.isEmpty(), "DbDataSource::insert", "insert_sql cannot be empty");

    QSqlQuery insert_query(database());
    if (!insert_query.prepare(insert_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, insert_query.lastError(), insert_sql);

    // Bind in the data parameters and execute the query
    for (int i=0, z=insert_fields.count(); i< z; ++i)
        insert_query.bindValue(i, dataRow.value(insert_fields.at(i)));

    if (!insert_query.exec())
        throw DatabaseError(constants::kMessageErrorExecutingQuery, insert_query.lastError(), insert_sql);

    // Database query executed successfully. If user did not supply a non-null id to be inserted, set the dataRow id
    // to the last insert id; otherwise, use the value supplied by the user. DbDataSource has direct access to the
    // DataRow private data because it is a friend class.
    if (dataRow.isEmpty()
        || !fields.contains("id")
        || dataRow.value("id").isNull())
    {
        dataRow.id_ = insert_query.lastInsertId();
    }
    else // dataRow has at least one value, contains a field called id, which has a non-null value
    {
        dataRow.id_ = dataRow.value("id");
    }

    return true;
}

/**
  * Continues inserting each DataRow until all have been successfully inserted or an error occurs. On failure,
  * the state of all rows previously inserted will depend on the external database setup (transasctions). No
  * care is taken to wrap this method call in a transaction or similar failsafe approach - that is the
  * responsibility of the caller.
  *
  * Returns the number of rows successfully inserted.
  *
  * @param dataRows [QList<DataRow> &]
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @returns int
  * @see insert(DataRow &dataRow, const QStringList &fields)
  *
  * @throws InvalidConnectionError, DatabaseError
  */
int DbDataSource::insert(QList<DataRow> &dataRows, const QString &tableName, const QStringList &fields) const
{
    checkTable(tableName);

    // Trivial case: no data to insert
    if (dataRows.isEmpty())
        return 0;

    // Build the insert SQL
    QStringList insert_fields = (fields.isEmpty()) ? getTableFields(tableName) : fields;
    QString insert_sql = buildInsertSql(tableName, insert_fields);

    // @see insert(DataRow &dataRow, const QStringList &fields) for logic regarding this statement
    Q_ASSERT_X(!insert_sql.isEmpty(), "DbDataSource::insert", "insert_sql cannot be empty");

    QSqlQuery insert_query(database());
    if (!insert_query.prepare(insert_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, insert_query.lastError(), insert_sql);

    int nInserted = 0;
    for (int i=0, z=dataRows.size(); i<z; ++i)
    {
        DataRow &dataRow = dataRows[i];

        // Cannot permit inserting any row that has a defined id_ member
        if (dataRow.id_.isValid())
            return nInserted;

        // Bind in the data parameters and execute the query
        for (int j=0, y=insert_fields.count(); j< y; ++j)
            insert_query.bindValue(j, dataRow.value(insert_fields.at(j)));

        // Only potential problem is that with this approach, we don't know how many were inserted
        if (!insert_query.exec())
            throw DatabaseError(constants::kMessageErrorExecutingQuery, insert_query.lastError(), insert_sql);

        // Database query executed successfully. If user did not supply a non-null id to be inserted, set the dataRow id
        // to the last insert id; otherwise, use the value supplied by the user.
        if (dataRow.isEmpty()
            || !fields.contains("id")
            || dataRow.value("id").isNull())
        {
            dataRow.id_ = insert_query.lastInsertId();
        }
        else // dataRow has at least one value, contains a field called id, which has a non-null value
        {
            dataRow.id_ = dataRow.value("id");
        }

        ++nInserted;
    }

    return nInserted;
}

/**
  * If fields is not defined, all fields from tableName will be read into a DataRow. Otherwise, will attempt
  * to read the fields specified from the tableName. If an invalid field is present, query preparation and
  * execution will not work properly and will throw a DatabaseError.
  *
  * @param tableName [const QString &]
  * @param id [int]
  * @param fields [const QStringList &]
  * @param conditions [const QString &]
  * @returns DataRow
  *
  * @throws InvalidConnectionError, DatabaseError
  */
DataRow DbDataSource::read(const QString &tableName, QVariant id, const QStringList &fields, const QString &conditions) const
{
    checkTable(tableName);

    if (!id.isValid())
        return DataRow();

    QStringList select_fields = (fields.isEmpty()) ? getTableFields(tableName) : fields;
    Q_ASSERT_X(!select_fields.isEmpty(), "DbDataSource::read", "select_fields must not be empty");

    // -------------------------------------
    // Build the SQL query
    QString select_sql = QString("SELECT %1 FROM %2 WHERE id = ?").arg(select_fields.join(", "), tableName);
    if (!conditions.isEmpty())
        select_sql += " AND " % conditions;

    // ------------------------------------
    // Prepare the query
    QSqlQuery select_query(database());
    if (!select_query.prepare(select_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, select_query.lastError(), select_sql);

    // ------------------------------------
    // Bind the id and execute
    select_query.bindValue(0, id);
    if (!select_query.exec())
        throw DatabaseError(constants::kMessageErrorExecutingQuery, select_query.lastError(), select_sql);

    if (select_query.next())
        return DataRow(id, select_query.record());

    return DataRow();
}

/**
  * Constructs a SQL query that selects fields from tableName where conditions and ordered by orderBy limited to limit rows.
  * If orderBy.fieldName is empty, then no explicit ordering is performed. Similarly, if limit is <= 0, then the limit clause
  * is excluded.
  *
  * Requires that tableName have an id field, which should be the primary key as well (although this is not enforced).
  *
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @param conditions [const QString &]
  * @param orderBy [const QPair<QString, SortDirection> &]
  * @param limit [int]
  * @returns QList<DataRow>
  *
  * @throws InvalidConnectionError, DatabaseError
  */
QList<DataRow> DbDataSource::readAll(const QString &tableName, const QStringList &fields, const QString &conditions, const OrderedField &orderBy, int limit) const
{
    checkTable(tableName);

    QStringList select_fields = (fields.isEmpty()) ? getTableFields(tableName) : fields;
    Q_ASSERT_X(!select_fields.isEmpty(), "DbDataSource::readAll", "select_fields must not be empty");

    // -------------------------------------
    // Build the SQL query
    QString select_sql = QString("SELECT id, %1 FROM %2").arg(select_fields.join(", "), tableName);
    if (!conditions.isEmpty())
        select_sql += " WHERE " % conditions;

    if (!orderBy.fieldName_.isEmpty())
        select_sql += QString(" ORDER BY %1 %2").arg(orderBy.fieldName_).arg(orderBy.sortDirection_ == eSortAscending ? "ASC" : "DESC");

    if (limit > 0)
        select_sql += QString(" LIMIT %1").arg(limit);

    // ------------------------------------
    // Prepare the query
    QSqlQuery select_query(database());
    if (!select_query.prepare(select_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, select_query.lastError(), select_sql);

    if (!select_query.exec())
        throw DatabaseError(constants::kMessageErrorExecutingQuery, select_query.lastError(), select_sql);

    // Iterate through database results and return list of DataRows
    QList<DataRow> dataRows;
    while (select_query.next())
    {
        QSqlRecord result = select_query.record();
        QVariant id = result.value("id");

        // Remove the id field, which we injected into the select_sql for the purpose of associating it with DataRow
        result.remove(0);

        dataRows.append(DataRow(id, result));
    }

    return dataRows;
}

/**
  * Releases the savepoint, name, via RELEASE SAVEPOINT {name}. All other savepoints created after this savepoint
  * was created are no longer valid. Name must be a database valid, non-empty string or a DatabaseError will be
  * thrown.
  *
  * @param name [const QString &]
  */
void DbDataSource::releaseSavePoint(const QString &name) const
{
    if (!databaseReady())
        throw InvalidConnectionError(constants::kMessageInvalidConnection, connectionName_);

    QSqlQuery releaseSavePointQuery(database());
    if (!releaseSavePointQuery.exec(QString("RELEASE SAVEPOINT %1").arg(name)))
        throw DatabaseError(constants::kMessageErrorExecutingQuery, releaseSavePointQuery.lastError(), QString("RELEASE SAVEPOINT %1").arg(name));
}

/**
  * To reset the sequence in a sqlite database table, it is necessary to delete this record
  * from the sqlite_sequence table. Does nothing if the database driver is not sqlite. Note, that due to the sqlite
  * implementation, the sequence will only be removed if there are no records in the table. Otherwise, the sequence
  * defaults to the highest record identifier + 1.
  *
  * @param tableName [const QString &]
  *
  * @throws DatabaseError
  */
void DbDataSource::resetSqliteSequence(const QString &tableName) const
{
    checkTable(tableName);

    if (!isSqlite())
        return;

    QSqlQuery u_sequence(database());
    QString u_sequence_sql = "DELETE FROM sqlite_sequence WHERE name = ?";
    if (!u_sequence.prepare(u_sequence_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, u_sequence.lastError(), u_sequence_sql);

    u_sequence.bindValue(0, tableName);
    if (!u_sequence.exec())
        throw DatabaseError(constants::kMessageErrorExecutingQuery, u_sequence.lastError(), u_sequence_sql);
}

/**
  * Roll back all SQL commands that were executed after the savepoint {name} was created. All savepoints created
  * after this savepoint are no longer valid. The savepoint still remains valid after this command. Name must be
  * a database valid, non-empty string or a DatabaseError will be thrown.
  *
  * @param name [const QString &]
  */
void DbDataSource::rollbackToSavePoint(const QString &name) const
{
    if (!databaseReady())
        throw InvalidConnectionError(constants::kMessageInvalidConnection, connectionName_);

    QSqlQuery rollbackSavePointQuery(database());
    if (!rollbackSavePointQuery.exec(QString("ROLLBACK TO SAVEPOINT %1").arg(name)))
        throw DatabaseError(constants::kMessageErrorExecutingQuery, rollbackSavePointQuery.lastError(), QString("ROLLBACK TO SAVEPOINT %1").arg(name));
}

/**
  * Begins a new savepoint with the SQL code: SAVEPOINT {name} and throws a database error if unable to perform
  * this action. Name must be a database valid, non-empty string or a DatabaseError will be thrown.
  *
  * @param name [const QString &]
  */
void DbDataSource::savePoint(const QString &name) const
{
    if (!databaseReady())
        throw InvalidConnectionError(constants::kMessageInvalidConnection, connectionName_);

    QSqlQuery savepointQuery(database());
    if (!savepointQuery.exec(QString("SAVEPOINT %1").arg(name)))
        throw DatabaseError(constants::kMessageErrorExecutingQuery, savepointQuery.lastError(), QString("SAVEPOINT %1").arg(name));
}

/**
  * Returns false if:
  * 1) dataRow.id_ is null
  * 2) dataRow does not contain any fields
  * 3) 0 rows were updated
  *
  * o fields must either be empty or completely be a subset of the database fields
  *
  * Update does not change any of the field values for DataRow, but in the event that the id is updated in the database
  * then the dataRow.id_ member is updated accordingly.
  *
  * If fields is empty, then all fields in dataRow will be saved to the database. Returns false if one or more fields
  * in dataRow is not present within the database table (because the query will fail). Thus, it is important to specify
  * at least one field if only some of the fields in dataRow should be considered for updating.
  *
  * Only returns true if a row was affected and at least one field was updated.
  *
  * @param dataRow [DataRow &]
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @param conditions [const QString &]
  * @returns bool
  * @see update(QList<DataRow> &dataRows, const QString &tableName, const QStringList &fields, const QString &conditions)
  *
  * @throws InvalidConnectionError, DatabaseError
  */
bool DbDataSource::update(DataRow &dataRow, const QString &tableName, const QStringList &fields, const QString &conditions) const
{
    checkTable(tableName);

    // Trivial case: no data to update
    if (dataRow.id_.isNull() || dataRow.isEmpty())
        return false;

    // -------------------------------------
    // Build and prepare the SQL query
    QStringList update_fields = (fields.isEmpty()) ? dataRow.fieldNames() : fields;
    QString update_sql = buildUpdateSql(tableName, update_fields, conditions);
    Q_ASSERT_X(!update_sql.isEmpty(), "DbDataSource::update", "update_sql must not be empty - check the fields argument");

    QSqlQuery update_query(database());
    if (!update_query.prepare(update_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, update_query.lastError(), update_sql);

    for (int i=0, z=update_fields.count(); i< z; ++i)
        update_query.bindValue(i, dataRow.value(update_fields.at(i)));
    update_query.bindValue(update_fields.count(), dataRow.id_);

    // -------------------------------------
    // Execute the query
    if (!update_query.exec())
        throw DatabaseError(constants::kMessageErrorExecutingQuery, update_query.lastError(), update_sql);

    if (update_query.numRowsAffected() > 0)
    {
        // Update the id if necessary
        if (update_fields.contains("id"))
            dataRow.id_ = dataRow.value("id");

        return true;
    }

    return false;
}

/**
  * Continues updating each DataRow until all have been successfully updated or an error occurs. On failure,
  * the state of all rows previously updated depends on the external database configuration. No care is taken
  * to wrap this method call in a transaction or similar failsafe approach - that is the responsibility of the
  * caller.
  *
  * Assumes that all dataRows have the same fields, if they do not then will cease updating and return the number
  * of records updated successfully to this point.
  *
  * Requirements for each dataRow:
  * o dataRow.id_ must not be null
  * o dataRow must have at least one field
  * o fields must either be empty or completely be a subset of the database fields
  * o Must at least have the fields defined in the first dataRow
  *
  * Returns the number of rows successfully updated.
  *
  * @param dataRows [const QList<DataRow> &]
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @param conditions [const QString &]
  * @returns int
  * @see update(DataRow &dataRow, const QString &tableName, const QStringList &fields, const QString &conditions)
  *
  * @throws InvalidConnectionError, DatabaseError
  */
int DbDataSource::update(QList<DataRow> &dataRows, const QString &tableName, const QStringList &fields, const QString &conditions) const
{
    checkTable(tableName);

    // Trivial case: no data to update
    if (dataRows.isEmpty())
        return 0;

    // Build the update SQL
    QStringList update_fields = (fields.isEmpty()) ? dataRows.at(0).fieldNames() : fields;
    QString update_sql = buildUpdateSql(tableName, update_fields, conditions);
    Q_ASSERT_X(!update_sql.isEmpty(), "DbDataSource::update (many)", "update_sql must not be empty - check the fields argument");

    QSqlQuery update_query(database());
    if (!update_query.prepare(update_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, update_query.lastError(), update_sql);

    int nUpdated = 0;
    for (int i=0, z=dataRows.size(); i<z; ++i)
    {
        DataRow &dataRow = dataRows[i];

        // Cannot permit inserting any row that has a defined id_ member
        if (dataRow.id_.isNull())
            return nUpdated;

        // Bind in the data parameters and execute the query
        for (int j=0, y=update_fields.count(); j< y; ++j)
        {
            // Each data row must have at least the fields specified in update_fields
            if (!dataRow.contains(update_fields.at(j)))
                return nUpdated;

            update_query.bindValue(j, dataRow.value(update_fields.at(j)));
        }
        update_query.bindValue(update_fields.count(), dataRow.id_);

        if (!update_query.exec())
            throw DatabaseError(constants::kMessageErrorExecutingQuery, update_query.lastError(), update_sql);

        // Database query executed successfully. If user did not supply a non-null id to be updated, set the dataRow id
        // to the last insert id; otherwise, use the value supplied by the user.
        if (update_fields.contains("id"))
            dataRow.id_ = dataRow.value("id");

        nUpdated += update_query.numRowsAffected();
    }

    return nUpdated;
}

/**
  * Update the database connection name to connectionName and confirm that the database is ready with this
  * connection unless it is an empty string. Connection name is only updated if databaseReady() returns true.
  *
  * @param connectionName [const QString &]
  *
  * @throws InvalidConnectionError
  */
void DbDataSource::setConnectionName(const QString &connectionName)
{
    QString oldConnectionName = connectionName_;
    connectionName_ = connectionName;

    if (connectionName.isEmpty() == false && !databaseReady())
    {
        connectionName_ = oldConnectionName;
        throw InvalidConnectionError(constants::kMessageInvalidConnection, connectionName);
    }
}

/**
  * Returns false if tableName does not exist or an error occurred while deleting all rows from tableName. Returns
  * true if operation is successful even if there are no rows in tableName.
  *
  * @param tablename [const QString &]
  * @returns bool
  *
  * @throws InvalidConnectionError, DatabaseError
  */
void DbDataSource::truncate(const QString &tableName) const
{
    checkTable(tableName);

    // Special rule for SQLite
    QString truncate_sql = (isSqlite()) ? "DELETE FROM " : "TRUNCATE ";
    truncate_sql += tableName;

    QSqlQuery truncate_query(database());
    if (!truncate_query.prepare(truncate_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, truncate_query.lastError(), truncate_sql);

    if (!truncate_query.exec())
        throw DatabaseError(constants::kMessageErrorExecutingQuery, truncate_query.lastError(), truncate_sql);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @returns QString
  */
QString DbDataSource::buildInsertSql(const QString &tableName, const QStringList &fields) const throw()
{
    if (fields.isEmpty())
        return "";

    return "INSERT INTO " % tableName % "(" % fields.join(", ") % ") VALUES (?" % QString(", ?").repeated(fields.count() - 1) % ")";
}

/**
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @param conditions [const QString &]
  * @returns QString
  */
QString DbDataSource::buildUpdateSql(const QString &tableName, const QStringList &fields, const QString &conditions) const throw()
{
    if (fields.isEmpty())
        return "";

    QString update_sql = QString("UPDATE %1 SET %2 = ? WHERE id = ?").arg(tableName, fields.join(" = ?, "));
    if (!conditions.isEmpty())
        update_sql += " AND " % conditions;

    return update_sql;
}

/**
  * @returns bool
  */
bool DbDataSource::isSqlite() const throw()
{
    return database().driverName() == "QSQLITE";
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param tableName [const QString &]
  * @returns QStringList
  */
QStringList DbDataSource::getTableFields(const QString &tableName) const
{
    Q_ASSERT_X(!tableName.isEmpty(), "DbDataSource::getTableFields", "tableName must not be empty; exception should have been thrown before here");
    Q_ASSERT_X(database().isOpen(), "DbDataSource::getTableFields", "Database must already be open; exception should have been thrown before here");

    // Release mode checks
    if (tableName.isEmpty() || database().isOpen() == false)
        return QStringList();

    QStringList fields;
    QSqlRecord tableSpec = database().record(tableName);
    for (int i=0, z=tableSpec.count(); i<z; ++i)
        fields << tableSpec.fieldName(i);

    return fields;
}

/**
  *
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @returns QStringList
  */
QStringList DbDataSource::getValidFields(const QString &tableName, const QStringList &fields) const
{
    QStringList table_fields = getTableFields(tableName);
    if (table_fields.isEmpty() || fields.isEmpty())
        return table_fields;

    // Find the intersection of fields between what the user supplied and is present in the database
    // add in each of the user-specified fields
    QStringList valid_fields;
    foreach (QString field, fields)
        if (table_fields.contains(field))
            valid_fields << field;

    return valid_fields;
}
