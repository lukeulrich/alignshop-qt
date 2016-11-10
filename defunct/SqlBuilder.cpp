/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SqlBuilder.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QStringList>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public static methods
/**
  * Example: tableName = amino_subseqs, fields = [id, label, start, stop]
  * Result: INSERT INTO amino_subseqs (id, label, start, stop) VALUES (?, ?, ?, ?)
  *
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @returns QString
  */
QString SqlBuilder::preparedInsertSql(const QString &tableName, const QStringList &fields)
{
    if (tableName.trimmed().isEmpty() || fields.isEmpty())
        return "";

    foreach (const QString &field, fields)
        if (field.trimmed().isEmpty())
            return "";

    return "INSERT INTO " % tableName.trimmed() % "(" % fields.join(", ") % ") VALUES (?" % QString(", ?").repeated(fields.count() - 1) % ")";
}

/**
  * Example: tableName = amino_subseqs, fields = [id, label, start, stop], conditions = "id = ?"
  * Result: UPDATE amino_subseqs SET id = ?, label = ?, start = ?, stop = ? WHERE id = ?
  *
  * Conditions may be empty and will be prefixed with WHERE.
  *
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @param conditions [const QString &]
  * @returns QString
  */
QString SqlBuilder::preparedUpdateSql(const QString &tableName, const QStringList &fields, const QString &conditions)
{
    if (tableName.trimmed().isEmpty() || fields.isEmpty())
        return "";

    foreach (const QString &field, fields)
        if (field.trimmed().isEmpty())
            return "";

    QString update_sql = QString("UPDATE %1 SET %2 = ?").arg(tableName.trimmed(), fields.join(" = ?, "));
    if (!conditions.isEmpty())
        update_sql += " WHERE " % conditions;

    return update_sql;
}


