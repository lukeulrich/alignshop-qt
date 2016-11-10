/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DbSpec.h"

#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QIODevice>
#include <QtCore/QStringBuilder>

#include <QtXmlPatterns/QXmlSchemaValidator>
#include <QtXmlPatterns/QXmlSchema>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Examines the loaded XML database specification instance for the following logical errors:
  * o duplicate table names
  * o duplicate field names
  * o duplicate index names
  * o autoincrement on text, real, or blobs
  * o multiple integer autoincrement fields
  * o autoincrement without also being defined as a primary key (SQLite-specific issue)
  * o duplicate primary key fields
  * o primary key field that does not exist
  * o duplicate foreign key source fields
  * o duplicate foreign key reference field
  * o foreign key source field does not exist
  * o foreign key reference table does not exist (this must be defined prior to the current table)
  * o foreign key reference field does not exist in referenced table
  * o number of foreign key source fields does not equal the number of foreign key reference fields
  * o duplicate index fields
  * o indexed field does not exist
  *
  * A string representation of these logical errors is passed to the message handler if one is defined.
  *
  * Returns false if no XML database instance has been loaded.
  *
  * @returns bool
  */
bool DbSpec::isLogicallyValid() const
{
    if (!isLoaded())
        return false;

    // A loaded specification indicates that it is both well-formed and adheres to a particular format
    // ASSUME: specific schema used to validate this XML instance. This function will fail royally if the schema
    //         does not have the expected format (e.g. table, field nodes, etc.)

    QHash<QString, bool> tables;
    QHash<QString, bool> indices;

    // Clear any previous error
    error(QString());

    // Walk through all tables
    QDomNodeList table_nodes = ddl_.elementsByTagName("table");
    for (int i=0, z=table_nodes.count(); i<z; ++i)
    {
        const QDomElement table_node = table_nodes.at(i).toElement();
        QString table_name = table_node.attribute("name");

        // Check that table is unique
        if (tables.contains(table_name))
        {
            error("[Logical error] duplicate table name: " % table_name);
            return false;
        }

        tables[table_name] = true;

        QString autoincrement_field;
        bool has_autoincrement = false;
        bool has_primary_key = false;
        QDomElement child = table_node.firstChildElement();
        while (!child.isNull())
        {
            if (child.tagName() == "field")
            {
                const QString field_name = child.attribute("name");
                const QString qualified_field_name = table_name % '.' % field_name;

                // -----------------------
                // Unique table.field name
                if (tables.contains(qualified_field_name))
                {
                    error("[Logical error] duplicate field name: " % qualified_field_name);
                    return false;
                }

                // -------------
                // Autoincrement
                const QString type = child.attribute("type");
                if (child.hasAttribute("autoincrement"))
                {
                    if (type != "integer" &&
                        type != "integer unsigned")
                    {
                        error(QString("[Logical error] autoincrement is only valid for integer-type fields (%1)").arg(qualified_field_name));
                        return false;
                    }

                    if (has_autoincrement)
                    {
                        error(QString("[Logical error] table may not have multiple autoincrement fields (%1)").arg(qualified_field_name));
                        return false;
                    }

                    has_autoincrement = true;

                    // Save the field that is tagged as an auto_increment'ing so that we can check it against the primary key
                    autoincrement_field = field_name;
                }

                tables[qualified_field_name] = true;
            }
            else if (child.tagName() == "primaryKey")
            {
                has_primary_key = true;

                // ---------------------------------
                // Unique list of primary key fields
                QStringList primary_key_fields = child.text().split(QRegExp("\\s*,\\s*"));
                if (primary_key_fields.removeDuplicates())
                {
                    error(QString("[Logical error] found duplicate primary key field (%1)").arg(table_name));
                    return false;
                }

                // -------------------------------------------
                // Each primary key is a validly defined field
                foreach (QString field_name, primary_key_fields)
                {
                    if (!tables.contains(table_name % '.' % field_name))
                    {
                        error(QString("[Logical error] primary key field (%1) does not exist in table (%2)").arg(field_name).arg(table_name));
                        return false;
                    }
                }

                // -------------------------------------------
                // auto_increment and primary key agree
                if (has_autoincrement)
                {
                    if (primary_key_fields.count() > 1)
                    {
                        error(QString("[Logical error] multi-field primary key (%1) and auto_increment field (%2) are not allowed to co-exist (table: %3)").arg(primary_key_fields.join(", ")).arg(autoincrement_field).arg(table_name));
                        return false;
                    }

                    Q_ASSERT_X(!autoincrement_field.isEmpty(), "DbSpec::isLogicallyValid", "has_auto_increment = true, but auto_increment_field is empty");

                    if (autoincrement_field != primary_key_fields.at(0))
                    {
                        error(QString("[Logical error] auto_increment field (%1) must be defined as the sole primary key or removed (table: %2)").arg(autoincrement_field).arg(table_name));
                        return false;
                    }
                }
            }
            else if (child.tagName() == "foreignKey")
            {
                // ----------------------------
                // Unique list of source fields
                const QString src_fields = child.firstChildElement("srcFields").text();
                QStringList foreign_key_src_fields = src_fields.split(QRegExp("\\s*,\\s*"));
                if (foreign_key_src_fields.removeDuplicates())
                {
                    error(QString("[Logical error] found duplicate foreign key source field(s) (%1): %2").arg(table_name).arg(src_fields));
                    return false;
                }

                // -------------------------------
                // Unique list of reference fields
                const QString ref_fields = child.firstChildElement("refFields").text();
                QStringList foreign_key_ref_fields = ref_fields.split(QRegExp("\\s*,\\s*"));
                if (foreign_key_ref_fields.removeDuplicates())
                {
                    error(QString("[Logical error] found duplicate foreign key reference field(s) (%1): %2").arg(table_name).arg(ref_fields));
                    return false;
                }

                // ------------------------------------------------
                // Each source key field is a validly defined field
                foreach (QString field_name, foreign_key_src_fields)
                {
                    if (!tables.contains(table_name % '.' % field_name))
                    {
                        error(QString("[Logical error] foreign key source field (%1) does not exist in table (%2)").arg(field_name).arg(table_name));
                        return false;
                    }
                }

                // ---------------------------
                // Referenced table must exist
                QString referenced_table_name = child.firstChildElement("refTable").text();
                if (!tables.contains(referenced_table_name))
                {
                    error(QString("[Logical error] referenced table (%1) does not exist").arg(referenced_table_name));
                    return false;
                }

                // ---------------------------
                // Referenced field must exist
                foreach (QString field_name, foreign_key_ref_fields)
                {
                    if (!tables.contains(referenced_table_name % '.' % field_name))
                    {
                        error(QString("[Logical error] foreign key source field (%1) does not exist in table (%2)").arg(field_name).arg(referenced_table_name));
                        return false;
                    }
                }

                // ---------------------------------------
                // Equal number of source reference fields
                if (foreign_key_src_fields.count() != foreign_key_ref_fields.count())
                {
                    error(QString("[Logical error] unequal number of foreign key source and reference fields: %1 vs %2").arg(src_fields).arg(ref_fields));
                    return false;
                }
            }
            else if (child.tagName() == "unique")
            {
                // ---------------------------------
                // Unique list of primary key fields
                QStringList unique_key_fields = child.text().split(QRegExp("\\s*,\\s*"));
                if (unique_key_fields.removeDuplicates())
                {
                    error(QString("[Logical error] found duplicate unique key field (%1)").arg(table_name));
                    return false;
                }

                // -------------------------------------------
                // Each unique field must reference a validly defined field
                foreach (QString field_name, unique_key_fields)
                {
                    if (!tables.contains(table_name % '.' % field_name))
                    {
                        error(QString("[Logical error] unique key field (%1) does not exist in table (%2)").arg(field_name).arg(table_name));
                        return false;
                    }
                }
            }
            else if (child.tagName() == "index")
            {
                const QString index_name = child.attribute("name");
                const QString qualified_index_name = table_name % '.' % index_name;

                // Check that the index name is unique
                if (indices.contains(qualified_index_name))
                {
                    error("[Logical error] duplicate index name: " % qualified_index_name);
                    return false;
                }

                indices[qualified_index_name] = true;

                // -----------------------------
                // Unique list of indexed fields
                QStringList indexed_fields = child.text().split(QRegExp("\\s*,\\s*"));
                if (indexed_fields.removeDuplicates())
                {
                    error(QString("[Logical error] found duplicate indexed field(s) (%1): %2").arg(table_name).arg(indexed_fields.join(", ")));
                    return false;
                }

                // -----------------------------
                // Each indexed field must exist
                foreach (QString field_name, indexed_fields)
                {
                    if (!tables.contains(table_name % '.' % field_name))
                    {
                        error(QString("[Logical error] indexed field (%1) does not exist in table (%2)").arg(field_name).arg(table_name));
                        return false;
                    }
                }
            }

            child = child.nextSiblingElement();
        }

        // Check that if there is an auto_increment field that a primary key is also defined (we already check that it
        // the primary key is the auto_increment field)
        if (has_autoincrement && !has_primary_key)
        {
            error(QString("[Logical error] no primary key defined, yet table (%1) has an auto_increment field (%2)").arg(table_name).arg(autoincrement_field));
            return false;
        }

    } // for each table node

    return true;
}

/**
  * After ensuring that xmlSchemaFile contains a valid W3 XML Schema, validates that xmlSpecFile is a valid XML instance.
  * Given a valid XML instance, load it into ddlDomDocument_. Returns true on success.
  *
  * @param xmlSchemaFileName [const QString &]
  * @param xmlSpecFileName [const QString &]
  * @returns bool
  */
bool DbSpec::load(const QString &xmlSchemaFileName, const QString &xmlSpecFileName)
{
    // Clear any previously loaded document and error message
    ddlDomDocument_.setContent(QString());
    ddl_ = ddlDomDocument_.documentElement();
    error("");

    // -------------------
    // Load the XML schema
    QFile xmlSchemaFile(xmlSchemaFileName);
    if (!xmlSchemaFile.open(QIODevice::ReadOnly))
    {
        error(QString("Unable to open XML schema file '%1'").arg(xmlSchemaFileName));
        return false;
    }

    // Read in the schema and ensure it is valid
    QXmlSchema dbSchema;
    dbSchema.load(&xmlSchemaFile, QUrl::fromLocalFile(xmlSchemaFile.fileName()));
    if (!dbSchema.isValid())
    {
        error(QString("XML schema document '%1' is not valid").arg(xmlSchemaFileName));
        return false;
    }
    xmlSchemaFile.close();

    // ----------------------------------
    // Load in the db schema XML instance
    QFile xmlSpecFile(xmlSpecFileName);
    if (!xmlSpecFile.open(QIODevice::ReadOnly))
    {
        error(QString("Unable to open XML specification file '%1'").arg(xmlSpecFile.fileName()));
        return false;
    }

    // -----------------------------------
    // Validate the db schema XML instance
    QXmlSchemaValidator xmlSchemaValidator(dbSchema);
    xmlSchemaValidator.setMessageHandler(handler_);
    if (!xmlSchemaValidator.validate(&xmlSpecFile, QUrl::fromLocalFile(xmlSpecFile.fileName())))
        return false;

    // --------------------------------------
    // Load the xml data into ddlDomDocument_
    xmlSpecFile.seek(0);
    if (!ddlDomDocument_.setContent(&xmlSpecFile))
    {
        error("Unable to set xml content");
        return false;
    }

    ddl_ = ddlDomDocument_.documentElement();

    return true;
}

/**
  * After ensuring that xmlSchema is a valid W3 XML Schema, validates that xmlStream is a valid XML instance. Given a
  * valid XML instance, load it into ddlDomDocument_. Will seek to the beginning of the xmlDevice before processing
  * the DOM.
  *
  * @param xmlSchema[QXmlSchema &]
  * @param xmlStream [QTextStream &]
  * @returns bool
  */
/* bool DbSpec::load(const QXmlSchema &xmlSchema, QIODevice *xmlDevice, const QUrl &documentUri)
{
    ddlDomDocument_.setContent(QString());
    ddl_ = ddlDomDocument_.documentElement();

    Q_ASSERT_X(xmlDevice, "DbSpec::load", "xmlDevice must not be null");
    if (!xmlDevice)
    {
        if (handler_)
            handler_->message(QtWarningMsg, "xmlDevice must not be null");
        return false;
    }

    if (!xmlSchema.isValid())
        return false;

    if (xmlDevice->bytesAvailable() == 0)
    {
        if (handler_)
            handler_->message(QtWarningMsg, "empty input");
        return false;
    }

    // Validate the xmlStream instance
    QXmlSchemaValidator xmlSchemaValidator(xmlSchema);
    xmlSchemaValidator.setMessageHandler(handler_);
    if (!xmlSchemaValidator.validate(xmlDevice, documentUri))
    {
        if (handler_)
            handler_->message(QtWarningMsg, "XML document is not valid");
        return false;
    }

    // Load the xml data into ddlDomDocument_
    xmlDevice->seek(0);
    if (!ddlDomDocument_.setContent(xmlDevice))
    {
        if (handler_)
            handler_->message(QtWarningMsg, "Unable to set xml content");
        return false;
    }

    ddl_ = ddlDomDocument_.documentElement();

    return true;
}
*/

/**
  * This outputs a single SQLite compatible string of code based on the currently loaded DDL that may be used to create
  * a SQLite database.
  *
  * [TODO: Implement test code]
  *
  * @returns QString
  * @see toSqlLlist()
  */
QString DbSpec::sql() const
{
    return sqlQueryList().join("\n");
}

/**
  * Builds a string list of individual SQLite compatible queries that collectively define this database specification.
  *
  * [TODO: Implement test code]
  *
  * @returns QStringList
  * @see toSql()
  */
QStringList DbSpec::sqlQueryList() const
{
    if (!isLogicallyValid())
        return QStringList();

    QStringList sql_queries;

    // -------
    // Pragmas
    QDomNodeList pragmas = ddl_.elementsByTagName("pragma");
    for (int i=0, z=pragmas.count(); i<z; ++i)
        sql_queries << pragmas.at(i).toElement().text();

    // ------
    // Tables
    QDomNodeList table_nodes = ddl_.elementsByTagName("table");
    for (int i=0, z=table_nodes.count(); i<z; ++i)
    {
        QStringList sql_lines;

        QDomElement table_node = table_nodes.at(i).toElement();
        sql_lines << "create table " % table_node.attribute("name");

        if (table_node.hasAttribute("comment"))
            sql_lines << "-- " % table_node.attribute("comment");

        sql_lines << "(";

        QString primaryKey;
        QStringList foreignKeys;
        QStringList uniques;
        QStringList checks;
        QStringList column_comments;
        QStringList columns;
        QStringList indices;

        // If auto_increment is set, then ignore the primary key field
        bool has_auto_increment = false;

        QDomElement child = table_node.firstChildElement();
        while (!child.isNull())
        {
            if (child.tagName() == "field")
            {
                QString column = "  " % child.attribute("name") % ' ' % child.attribute("type");
                if (child.hasAttribute("not_null") && child.attribute("not_null") == "true")
                    column += " not null";

                // Fact: logically valid specification file
                // Therefore: if auto_increment is present, it is the primary key
                if (child.hasAttribute("autoincrement") && child.attribute("autoincrement") == "true")
                {
                    column += " primary key autoincrement";
                    has_auto_increment = true;
                }

                if (child.hasAttribute("default"))
                    column += " default " % child.attribute("default");

                columns << column;

                if (child.hasAttribute("comment"))
                    column_comments << child.attribute("comment");
                else
                    column_comments << QString();
            }
            else if (child.tagName() == "primaryKey")
            {
                if (!has_auto_increment)
                    primaryKey = "  primary key(" % child.text() % ")";
            }
            else if (child.tagName() == "foreignKey")
            {
                QString foreignKey = QString("  foreign key(%1) references %2(%3) on update %4 on delete %5")
                        .arg(child.firstChildElement("srcFields").text())
                        .arg(child.firstChildElement("refTable").text())
                        .arg(child.firstChildElement("refFields").text())
                        .arg(child.attribute("onUpdate"))
                        .arg(child.attribute("onDelete"));

                foreignKeys.append(foreignKey);
            }
            else if (child.tagName() == "unique")
            {
                uniques.append(QString("  unique(%1)").arg(child.text()));
            }
            else if (child.tagName() == "check")
            {
                checks << QString("  check(%1)").arg(child.text());
            }
            else if (child.tagName() == "index")
            {
                indices << QString("create index %1 on %2(%3)")
                        .arg(child.attribute("name"))
                        .arg(table_node.attribute("name"))
                        .arg(child.text());
            }

            child = child.nextSiblingElement();
        }

        // Tricky case of adding in column comments and the last column:
        // o Case 1: The table only has column definitions - thus, no comma should terminate the last column
        // o Case 2: Table has column definitions and additional DDL commands - a comman should be placed after the
        //           the last column definition.
        // To handle these, we set a boolean flag which denotes whether there is additional DDL, which is determined
        // by the presence of a primary key, foreign key, or checks
        int other_ddl_lines = (!primaryKey.isEmpty()) ? 1 : 0;
        other_ddl_lines += foreignKeys.count() + uniques.count() + checks.count();

        // Find the longest length column definition
        int longest = 0;
        foreach (QString column, columns)
            if (column.length() > longest)
                longest = column.length();

        for (int i=0, z=columns.count(); i<z; ++i)
        {
            QString column_sql = columns.at(i);
            if (i != z-1 || other_ddl_lines)
                column_sql += ',';

            column_sql += QString(" ").repeated(longest - columns.at(i).length());

            // Add in any comments
            if (!column_comments.at(i).isEmpty())
                column_sql += "     -- " % column_comments.at(i);

            sql_lines << column_sql;
        }

        // Primary key
        if (!primaryKey.isEmpty())
        {
            sql_lines << "\n" % primaryKey;
            --other_ddl_lines;
            if (other_ddl_lines)
                sql_lines.last() += ',';
        }

        // Foreign keys
        if (foreignKeys.count())
        {
            for (int i=0, z=foreignKeys.count(); i<z; ++i)
            {
                sql_lines << foreignKeys.at(i);
                if (i != z-1)
                    sql_lines.last() += ',';
                --other_ddl_lines;
            }
            if (other_ddl_lines)
                sql_lines.last() += ',';
        }

        // Unique constraints
        if (uniques.count())
        {
            for (int i=0, z=uniques.count(); i<z; ++i)
            {
                sql_lines << uniques.at(i);
                if (i != z-1)
                    sql_lines.last() += ',';
                --other_ddl_lines;
            }
            if (other_ddl_lines)
                sql_lines.last() += ',';
        }

        // Check constraints
        if (checks.count())
        {
            for (int i=0, z=checks.count(); i<z; ++i)
            {
                sql_lines << checks.at(i);
                if (i != z-1)
                    sql_lines.last() += ',';
                --other_ddl_lines;
            }
        }

        Q_ASSERT_X(other_ddl_lines == 0, "DbSpec::sqlQueryList", "No more remaining DDL lines but count indicates that there is");

        // Terminate table definition
        sql_lines << ");";

        sql_queries << sql_lines.join("\n");

        // indices
        sql_queries << indices;
    }

    return sql_queries;
}
