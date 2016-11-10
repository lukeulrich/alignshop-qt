/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DATAROW_H
#define DATAROW_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include <QtSql/QSqlRecord>

class QStringList;

/**
  * DataRow is a lightweight, associative data structure for flexibly moving data that simply builds upon
  * QSqlRecord by adding a public id_ field.
  *
  * Essentially, DataRow models a single row from a data table (conceptual data table which may be an
  * actual database table, tsv sheet, or any other data source that lends itself to an simple associative
  * nature), which consists of a set of named fields and associated values.
  *
  * Each row is uniquely identified ("primary key") by a single id_ field, which is primarily utilized during
  * insert/updates.
  *
  * Because all members are implicitly shared objects, there is no need to explicitly make this class as a
  * whole implicitly shared. Moreover, because of this fact, the default copy constructor and assignment
  * operator will also function as desired.
  */
class DataRow : public QSqlRecord
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    explicit DataRow(const QVariant &id = QVariant());              //!< Construct an instance of this class with id
    DataRow(const QSqlRecord &sqlRecord);                           //!< Construct a copy of sqlRecord
    DataRow(const QVariant &id, const QSqlRecord &sqlRecord);       //!< Construct an instance of this class with id and a copy of sqlRecord

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QStringList fieldNames() const;                                 //!< Returns a list of field names associated with this DataRow
    using QSqlRecord::remove;
    void remove(const QString &fieldName);                          //!< Utility function for removing fieldName from this DataRow. If fieldName is not present, nothing happens

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    // To avoid masking the alternative setValue method of QSqlRecord
    using QSqlRecord::setValue;
    void setValue(const QString &fieldName, const QVariant &value); //!< Sets the value of fieldName (appending it if it does not exist) to value

    // ------------------------------------------------------------------------------------------------
    // Public members
    QVariant id_;                                        //!< Unique identiifer for this row
};

Q_DECLARE_METATYPE(DataRow)
Q_DECLARE_METATYPE(QList<DataRow>)

#endif // DATAROW_H
