/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DBSPEC_H
#define DBSPEC_H

#include <QtCore/QStringList>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <QtXmlPatterns/QAbstractMessageHandler>

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QAbstractMessageHandler;

/**
  * Loads and validates a database specification file encoded in XML.
  *
  * The need for a queryable database specification arose from the need to programmatically inspect and analyze a database
  * structure for referential integrity. In the context of an AlignShop document (SQLite database), it is possible for
  * the user to disable foreign keys and then put the records into an inconsistent state by deleting/inserting/updating
  * foreign key identifiers. Additionally, database documents are expected to have certain fields with specific types
  * and it is very possible for the user to modify these into an incompatible state when manipulating the database
  * with an external tool/interface.
  *
  * Thus, a DbSpec, or database specification, details the table structure, relationships, indices, and other constraints
  * corresponding to a particular database file.
  *
  * There are two types of validity:
  * 1) Since the DbSpec is encoded in XML, it must conform to an XML schema
  * 2) The XML itself may be valid; however, there may be logical errors within the database table relationships
  *
  * Detailed error handling is handled by via an instance of QAbstractMessageHandler *
  *
  * In the first case, the load function will return false and set the appropriate error message. The specification is
  * not saved. In the latter case, the load function will return true, but the user must call the isLogicallyValid function
  * to ensure that the expected database structure is present.
  *
  * Database schemas are not supported. All tables must only occur once in the specification and belong to the default
  * DBMS-specific table space (e.g. public schema in postgresql).
  *
  * The major components of a DbSpec are the XML schema which defines the expected structure of an XML instance and the
  * XML instance, or user document, that should conform to this XML schema. Because the XML schema and XML database
  * specification are purely internal documents, upfront testing should mitigate any errors when reading these files;
  * however, we double-check that they load properly at runtime.
  */
class DbSpec
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    DbSpec();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    const QDomElement ddl() const;                                  //!< Return the XML-encoded (QDomElement) data definition language for this specification
    bool isLoaded() const;                                          //!< Returns whether a specification has been loaded
    bool isLogicallyValid() const;                                  //!< Analyzes the specification for logical DDL errors and returns true if there are no errors, false otherwise
    //! Attempts to load the specification from xmlSpecFileName and ensures that it conforms to the schema defined in xmlSchemaFileName; returns true on success or false otherwise
    bool load(const QString &xmlSchemaFileName, const QString &xmlSpecFileName);
    void setMessageHandler(QAbstractMessageHandler *handler);       //!< Sets the message handler to handler
    QString sql() const;                                            //!< Outputs a SQLite compatible string of SQL code
    QStringList sqlQueryList() const;                               //!< Outputs a list of individual SQLite compatible queries that define this DbSpec

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void error(const QString &message) const;                       //!< Convenience function for pushing messages to any defined message handler

    QAbstractMessageHandler *handler_;          //!< Pointer to a message handler instance for recording messages
    QDomDocument ddlDomDocument_;               //!< The XML document instance for the database specification
    QDomElement ddl_;                           //!< The root element of the ddlDomDocument_
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
inline
DbSpec::DbSpec()
{
    handler_ = 0;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @return QScriptValue
  */
inline
const QDomElement DbSpec::ddl() const
{
    return ddl_;
}

/**
  * Simply checks for a non-empty ddl_
  *
  * @returns bool
  */
inline
bool DbSpec::isLoaded() const
{
    return !ddl_.isNull();
}

/**
  * @param handler [QAbstractMessageHandler *]
  */
inline
void DbSpec::setMessageHandler(QAbstractMessageHandler *handler)
{
    handler_ = handler;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Convenience routine for routing error messages.
  *
  * @params message [const QString &]
  */
inline
void DbSpec::error(const QString &message) const
{
    if (handler_)
        handler_->message(QtWarningMsg, message);
}

#endif // DBSPEC_H
