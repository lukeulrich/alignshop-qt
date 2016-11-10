/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CRUDSQLRECORD_H
#define CRUDSQLRECORD_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlField>
#include <QtSql/QSqlRecord>

#include <QtXmlPatterns/QAbstractMessageHandler>

#include "DbTable.h"

/**
  * The CrudSqlRecord extends QSqlRecord to provide a fluid and structured interface for standard CRUD
  * operations on a per-row basis.
  *
  * CRUD behaves as follows:
  * o To SELECT, simply call the read function and pass the record identifier (primary key) to fetch. Alternatively,
  *   it is possible to set the public id_ data member directly and call read without any arguments.
  * o To INSERT, setValue (via the QSqlRecord interface) for the relevant fields, and call save(). For this to
  *   work properly, id_ must be invalid or NULL.
  * o To UPDATE, set id_ to correspond to the relevant record, and then call save().
  * o To DELETE, set id_ and call erase()
  *
  * By default, all fields in the dbTable_ are utilized for all queries except DELETE. To utilize only a subset of all
  * fields, simply append, remove, etc. the desired fields.
  *
  * Because all CRUD operations ultimately relate to a DbTable database, specific error messages may be retrieved via
  * the QSqlDatabase object (e.g. dbTable_.database().lastError()).
  *
  * While all database systems are usually case-insensitive, these methods and members function in a case-senstive
  * manner. Thus, it is important to use a consistent naming scheme that is compliant with the source DBMS.
  *
  * TODO: If the source table does not have a single field primary key established in the database, the first table column
  *       will be defined as the operating primary key. After saving a record, the id_ field will not be properly updated
  *       because we have no way of getting the actual identifier.
  * THEREFORE: Only works properly as of now with auto-increment values as the primary key.
  *
  * Largely influenced by the CakePHP project.
  */
class CrudSqlRecord : public QSqlRecord
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    CrudSqlRecord(const DbTable &dbTable);                        //!< Constructs a representation of database table, tableName

    // ------------------------------------------------------------------------------------------------
    // Public methods
    // CRUD interface
    virtual int erase(const QVariant &id = QVariant());          //!< Deletes a single record from the database identified by id or id_ if id is NULL; returns the number of rows deleted on success, -1 if input error, or -2 on database error
    virtual int read(const QVariant &id = QVariant());           //!< Reads a single record identified by id or id_ if id is NULL; returns the number of rows read on success, -1 if input error, or -2 on database error
    //! Inserts (if id_ isEmpty) or updates (if id_ is not empty) the data stored in fields_; if insert, automatically sets id_ to the last_insert_id. returns the number of rows affected on success, -1 if input error, or -2 on database error
    virtual int save();

    // Utility functions
    virtual DbTable dbTable() const;                              //!< Returns the associated DbTable instance
    virtual void defaultFields();                                 //!< Reset fields to utilize all fields specified in dbTable_
    virtual bool hasData() const;                                 //!< Returns true if any fields have a non-NULL value; false otherwise
    virtual QStringList queryFields() const;                      //!< Returns a list of all currently set field names
    virtual void setDbTable(const DbTable &dbTable);              //!< Sets dbTable as the new reference table
    void setMessageHandler(QAbstractMessageHandler *handler);     //!< Sets the message handler to handler

    // ------------------------------------------------------------------------------------------------
    // Slightly modified versions of the append, insert, and replace QSqlRecord functions intended to constrain modifications
    // to those fields that exist within dbTable_. This is in contrast to QSqlRecord, which permits the modification of any
    // field regardless of its context.
    bool append(const QString &fieldName);                        //!< Append fieldName to this record's fields; returns true on success, false otherwise
    bool insert(int pos, const QString &fieldName);               //!< Insert fieldName at pos to this record's fields; returns true on success, false otherwise
    bool replace(int pos, const QString &fieldName);              //!< Replaces the field at pos with fieldName; returns true on success, false otherwise
    bool remove(const QString &fieldName);                        //!< Overloaded function to remove the field named fieldName; returns true on success, false otherwise

    // To include the overloaded QSqlRecord::remove(int) function
    using QSqlRecord::remove;


    // ------------------------------------------------------------------------------------------------
    // Public members
    QVariant id_;                           //!< Primary key identifier for this record

protected:
    // ------------------------------------------------------------------------------------------------
    // Converted QSqlRecord public functions not intended for actual use (and thus they have no definition)
    void append(const QSqlField &field);
    void insert(int pos, const QSqlField &field);
    void replace(int pos, const QSqlField &field);

    // ------------------------------------------------------------------------------------------------
    // CrudSqlRecord specific functions
    void error(const QString &message) const;                     //!< Convenience function for pushing messages to any defined message handler

    // ------------------------------------------------------------------------------------------------
    // Protected data members
    DbTable dbTable_;                       //!< Reference database table
    QAbstractMessageHandler *handler_;      //!< Pointer to a message handler instance for recording messages
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * CrudSqlRecord only supports reading rows that are uniquely identified by a single column. In strict
  * database terms this would mean that every table must have a single-field primary key; however, this
  * constraint is relaxed here and the mechanism used for identifying a database row is the field returned
  * by dbTable.primaryKeyField().
  *
  * If dbTable is empty, then all CRUD operations will return false.
  *
  * @param dbTable [const DbTable &]
  */
inline
CrudSqlRecord::CrudSqlRecord(const DbTable &dbTable) : QSqlRecord(dbTable), dbTable_(dbTable)
{
    handler_ = 0;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns DbTable
  */
inline
DbTable CrudSqlRecord::dbTable() const
{
    return dbTable_;
}

/**
  */
inline
void CrudSqlRecord::defaultFields()
{
    // Remove all columns from this record
    clear();

    // Append each field listed in the dbTable_ record object
    for (int i=0, z=dbTable_.count(); i<z; ++i)
        QSqlRecord::append(dbTable_.field(i));
}

/**
  */
inline
bool CrudSqlRecord::hasData() const
{
    for (int i=0, z=count(); i<z; ++i)
        if (isNull(i) == false)
            return true;

    return false;
}

/**
  * Returns a list of all field names associated with this record.
  *
  * @returns QStringList
  */
inline
QStringList CrudSqlRecord::queryFields() const
{
    QStringList list;

    for (int i=0, z=count(); i<z; ++i)
        list << fieldName(i);

    return list;
}

/**
  * Sets the associated database table to dbTable and resets queryFields_ to include all fields in dbTable (if any). Also,
  * resets the id_ data member.
  *
  * @param dbTable [const DbTable &]
  * @returns bool
  */
inline
void CrudSqlRecord::setDbTable(const DbTable &dbTable)
{
    dbTable_ = dbTable;
    defaultFields();
    id_ = QVariant();
}

/**
  * @param handler [QAbstractMessageHandler *]
  */
inline
void CrudSqlRecord::setMessageHandler(QAbstractMessageHandler *handler)
{
    handler_ = handler;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplmemented QSqlRecord public methods
/**
  * Performs the same operation as QSqlRecord::append, except that fieldName must exist in dbTable_. If it does
  * and it is not already an active field in this record, append it and return true. Otherwise, return false.
  *
  * @param field [const QString &fieldName]
  * @returns bool
  * @see insert(), replace()
  */
inline
bool CrudSqlRecord::append(const QString &fieldName)
{
    if (contains(fieldName) || !dbTable_.contains(fieldName))
        return false;

    QSqlRecord::append(fieldName);

    return true;
}

/**
  * Performs the same operation as QSqlRecord::insert, except that fieldName must exist in dbTable_. If it does
  * and it is not already an active field in this record, insert it and return true. Otherwise, return false.
  *
  * @param pos [int]
  * @param fieldName [const QString &]
  * @returns bool
  * @see append(), replace()
  */
inline
bool CrudSqlRecord::insert(int pos, const QString &fieldName)
{
    if (contains(fieldName) || !dbTable_.contains(fieldName))
        return false;

    QSqlRecord::insert(pos, fieldName);

    return true;
}

/**
  * Performs the same operation as QSqlRecord::replace, except that fieldName must exist in dbTable_. If it does,
  * replace it and return true. Otherwise, return false. Replacing a field with itself also works, although this
  * does nothing.
  *
  * @param pos [int]
  * @param fieldName [const QString &]
  * @returns bool
  * @see append(), insert()
  */
inline
bool CrudSqlRecord::replace(int pos, const QString &fieldName)
{
    // Self-replacement does not require any changes, but should still return true
    if (fieldName == QSqlRecord::fieldName(pos))
        return true;

    if (contains(fieldName) || !dbTable_.contains(fieldName))
        return false;

    QSqlRecord::replace(pos, fieldName);

    return true;
}

/**
  * Removes fieldName from the list of fields, if it exists in dbTable_ and is currently an active field.
  *
  * @param fieldName [const QString &]
  * @returns bool
  */
inline
bool CrudSqlRecord::remove(const QString &fieldName)
{
    if (!dbTable_.contains(fieldName))
        return false;

    int pos = indexOf(fieldName);
    if (pos == -1)
        return false;

    remove(pos);

    return true;
}

/**
  * Convenience routine for routing error messages.
  *
  * @params message [const QString &]
  */
inline
void CrudSqlRecord::error(const QString &message) const
{
    if (handler_)
        handler_->message(QtWarningMsg, message);
}

#endif // CRUDSQLRECORD_H
