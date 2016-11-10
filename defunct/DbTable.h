/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DBTABLE_H
#define DBTABLE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlIndex>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

/**
  * DbTable encapsulates the metadata regarding a specific database table.
  *
  * DbTable extends QSqlRecord's ability to reflect a given table structure with the addition of a primary key field and
  * internally storing the table name. Upon construction, the table, tableName, associated with the database connection is
  * inspected and its structure initalized.
  *
  * Limitations:
  * o Only works with single-field primary keys; will simply utilize the field "id" or the first field (if table.id is not
  *   present) as the primary key
  *
  * Future:
  * o Provide capability to add calculated columns and aliases
  */
class DbTable : public QSqlRecord
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    DbTable(const QString &tableName, const QSqlDatabase &database);    //!< Construct a representation of tableName within database
    DbTable(const DbTable &other);                                      //!< Create a copy of other

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QSqlDatabase database() const;                                      //!< Return the database connection used to populate this record
    QString primaryKeyField() const;                                    //!< Returns a string representation of the primary key field
    virtual bool setPrimaryKeyField(const QString &fieldName);          //!< Set the primary key field to fieldName; returns true on success, or false otherwise
    //!< Load structure of tableName in database (or database_ if database is null), and clear any internal data; returns true on success. false otherwise
    virtual bool setTable(const QString &tableName, const QSqlDatabase &database = QSqlDatabase());
    virtual QString tableName() const;                                  //!< Returns tableName_

protected:
    // ------------------------------------------------------------------------------------------------
    // Converted QSqlRecord public functions; these are no longer available because they are not relevant operations
    void clearValues();
    void setValue(int index, const QVariant &val);
    void setValue(const QString &name, const QVariant &val);
    QVariant value(int index) const;
    QVariant value(const QString &name) const;

    // ------------------------------------------------------------------------------------------------
    // Protected data members
    QString tableName_;                     //!< Table name
    QString databaseName_;                  //!< Database connection name associated with this record
    QString primaryKeyField_;               //!< The key field that

private:
    // ------------------------------------------------------------------------------------------------
    // Private functions
    void _initializePrimaryKeyField();      //!< Helper function to set the primary key
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Qt uses QSqlRecord to represent a table structure, yet it does not contain the name of the source table or database, which
  * is a key goal for this class. Thus, upon initialization, a QSqlRecord representation of the table (via the QSqlDatabase::record())
  * function is invoked and then passed to the QSqlRecord base class for initialization.
  *
  * @param tableName [const QString &]
  * @param database [const QSqlDatabase &]
  */
inline
DbTable::DbTable(const QString &tableName, const QSqlDatabase &database) :
        QSqlRecord(database.record(tableName))
{
    if (!isEmpty())
    {
        tableName_ = tableName;
        databaseName_ = database.connectionName();
        _initializePrimaryKeyField();
    }
}

/**
  * @param other [const DbTable &]
  */
inline
DbTable::DbTable(const DbTable &other) :
       QSqlRecord(other.database().record(other.tableName_))
{
    if (!isEmpty())
    {
        tableName_ = other.tableName_;
        databaseName_ = other.databaseName_;
        primaryKeyField_ = other.primaryKeyField_;
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QSqlDatabase
  */
inline
QSqlDatabase DbTable::database() const
{
    return QSqlDatabase::database(databaseName_);
}

/**
  * @returns QString
  */
inline
QString DbTable::primaryKeyField() const
{
    return primaryKeyField_;
}

/**
  * Sets the primary key field for searching to fieldName if it is present in the database table and returns true.
  * Otherwise, returns false.
  *
  * @param fieldName [const QString &]
  * @returns bool
  */
inline
bool DbTable::setPrimaryKeyField(const QString &fieldName)
{
    if (contains(fieldName))
    {
        primaryKeyField_ = fieldName;
        return true;
    }

    return false;
}


/**
  * @returns QString
  */
inline
QString DbTable::tableName() const
{
    return tableName_;
}

#endif // DBTABLE_H
