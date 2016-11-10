/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DataRow.h"

#include <QtCore/QStringList>

#include <QtSql/QSqlField>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param tableName [const QString &]
  * @param id [const QVariant &]
  */
DataRow::DataRow(const QVariant &id) : QSqlRecord(), id_(id)
{
}

/**
  * Provides for initializing a DataRow from a SQL record
  *
  * @param sqlRecord [const QSqlRecord]
  */
DataRow::DataRow(const QSqlRecord &sqlRecord) : QSqlRecord(sqlRecord)
{
}

/**
  * Provides for constructing a DataRow with a specific id and SQL record
  *
  * @param id [const QVariant &]
  * @param sqlRecord [const QSqlRecord &]
  */
DataRow::DataRow(const QVariant &id, const QSqlRecord &sqlRecord) : QSqlRecord(sqlRecord), id_(id)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QStringList
  */
QStringList DataRow::fieldNames() const
{
    QStringList fields;
    for (int i=0, z=count(); i<z; ++i)
        fields << fieldName(i);

    return fields;
}

/**
  * @param fieldName [const QString &]
  */
void DataRow::remove(const QString &fieldName)
{
    QSqlRecord::remove(indexOf(fieldName));
}

/**
  * @param filedName [const QString &]
  * @param value [const QVariant &]
  */
void DataRow::setValue(const QString &fieldName, const QVariant &value)
{
    if (!contains(fieldName))
        QSqlRecord::append(fieldName);

    QSqlRecord::setValue(fieldName, value);
}
