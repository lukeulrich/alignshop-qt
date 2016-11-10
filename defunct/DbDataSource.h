/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DBDATASOURCE_H
#define DBDATASOURCE_H

#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>

#include "DataRow.h"
#include "global.h"

struct OrderedField
{
    QString fieldName_;
    SortDirection sortDirection_;

    OrderedField(const QString &fieldName = QString(), SortDirection sortDirection = eSortAscending) : fieldName_(fieldName), sortDirection_(sortDirection)
    {
    }
};

Q_DECLARE_TYPEINFO(OrderedField, Q_PRIMITIVE_TYPE);

/**
  * DbDataSource provides a CRUD interface and basic implementation for interacting with a relational database table
  * and exchanging data using DataRow objects.
  *
  * Client classes must provide a valid database connection name or else the majority of the methods in this class
  * will throw an InvalidConnectionError. Because all CRUD methods work with a specific table, a DatabaseError is thrown
  * if the table does not exist. Additionally, DatabaseError's are thrown whenever the SQL statements cannot be prepared
  * or executed properly (e.g. due to invalid conditions). Functions which do not throw an exception (either directly or
  * indirectly) contain the throw() clause.
  *
  * All target database tables must have a single primary key field called id (although this is not strictly enforced).
  *
  * All read and update methods accept an arbitrary where SQL clause in the form of a QString (conditions) for filtering
  * purposes. It is the client classes' responsibility to validate that this clause is well-formed and not subject to
  * SQL injection.
  *
  * If fields is empty, then all fields within the target table will be returned.
  *
  * All transactional steps should be handled externally.
  *
  * Future work:
  * >> Instead of requiring or asking for specific fields, request a list of integer positions
  */
class DbDataSource
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Construct an instance of this object with the database connection, connectionName
    explicit DbDataSource(const QString &connectionName = QString());
    virtual ~DbDataSource();                                    //!< Trivial destructor with virtual destructor for adequate subclass clean up

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void checkTable(const QString &tableName) const;            //!< Throws an InvalidConnectionError if invalid database connection or DatabaseError if tableName does not exist
    QString connectionName() const throw();                     //!< Returns the connection name associated with this DbDataSource
    QSqlDatabase database() const throw();                      //!< Utility function for retrieving the actual database handle
    bool databaseReady() const throw();                         //!< Utility function for checking if the database is valid and open
    //! Returns the number of records successfully deleted from tableName whose id field equals id and matched the SQL conditions; otherwise returns 0
    virtual int erase(const QString &tableName, QVariant id, const QString &conditions = QString()) const;


    // Add the field function:
//    virtual QVariant field(const QString &tableName, QVariant id, const QString &fieldName, const QString &conditions = QString()) const;


    //! Returns true if successfully able to insert dataRow fields into tableName or false otherwise
    virtual bool insert(DataRow &dataRow, const QString &tableName, const QStringList &fields = QStringList()) const;
    //! Executes insert for all dataRows and returns the number of rows successfully inserted
    virtual int insert(QList<DataRow> &dataRows, const QString &tableName, const QStringList &fields = QStringList()) const;
    //! On success, reads fields from tableName for the record identified by id and matching conditions and returns a DataRow with this information; otherwise returns an empty and uninitialized (talbename and id not defined) DataRow
    virtual DataRow read(const QString &tableName, QVariant id, const QStringList &fields = QStringList(), const QString &conditions = QString()) const;
    //! On success, reads all records from tableName with fields that match conditions and ordered by orderBy; returns limit DataRows (all rows if limit <= 0) with this information; otherwise returns an empty list
    virtual QList<DataRow> readAll(const QString &tableName,
                                   const QStringList &fields = QStringList(),
                                   const QString &conditions = QString(),
                                   const OrderedField &orderBy = OrderedField(),
                                   int limit = 0) const;
    void releaseSavePoint(const QString &name) const;           //!< Releases the save point with the given name
    void resetSqliteSequence(const QString &tableName) const;   //!< Sqlite-specific method for resetting the sequence of tableName
    void rollbackToSavePoint(const QString &name) const;        //!< Rollback to the save point with the given name
    void savePoint(const QString &name) const;                  //!< Begin a new save point with the given name
    void setConnectionName(const QString &connectionName);      //!< Sets the database connection name to connectionName
    virtual void truncate(const QString &tableName) const;      //!< Removes all records from tableName
    //! Updates fields in dataRow which match conditions and returns true on success or false otherwise
    virtual bool update(DataRow &dataRow, const QString &tableName, const QStringList &fields = QStringList(), const QString &conditions = QString()) const;
    //! Executes update for all dataRows and returns the number of rows successfully updated
    virtual int update(QList<DataRow> &dataRows,
                       const QString &tableName,
                       const QStringList &fields = QStringList(),
                       const QString &conditions = QString()) const;

protected:
    //! Returns a parameterized SQL insert string for inserting fields into tableName
    QString buildInsertSql(const QString &tableName, const QStringList &fields) const throw();
    //! Returns a parameterized SQL update string for updating fields within tableName that match conditions
    QString buildUpdateSql(const QString &tableName, const QStringList &fields, const QString &conditions = QString()) const throw();
    bool isSqlite() const throw();                              //!< Returns true if database driver is sqlite

private:
    //! Returns a list of fields contained in tableName
    QStringList getTableFields(const QString &tableName) const;
    //! Returns a list of valid field names that are present in tableName and fields
    QStringList getValidFields(const QString &tableName, const QStringList &fields) const;

    QString connectionName_;
};

#endif // DBDATASOURCE_H
