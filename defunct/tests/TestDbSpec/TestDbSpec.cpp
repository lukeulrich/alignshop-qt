/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QAbstractMessageHandler>

#include "DbSpec.h"

class MessageHandler : public QAbstractMessageHandler
 {
     public:
         MessageHandler()
             : QAbstractMessageHandler(0)
         {
         }

         QString statusMessage() const
         {
             return m_description;
         }

         int line() const
         {
             return m_sourceLocation.line();
         }

         int column() const
         {
             return m_sourceLocation.column();
         }

     protected:
         virtual void handleMessage(QtMsgType type, const QString &description,
                                    const QUrl &identifier, const QSourceLocation &sourceLocation)
         {
             Q_UNUSED(type);
             Q_UNUSED(identifier);

             m_messageType = type;
             m_description = description;
             m_sourceLocation = sourceLocation;
         }

     private:
         QtMsgType m_messageType;
         QString m_description;
         QSourceLocation m_sourceLocation;
 };

class TestDbSpec : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void load();            // Also tests isLoaded
    void setMessageHandler();
    void ddl();
    void isLogicallyValid();

    void sql();

private:
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
void TestDbSpec::constructor()
{
    DbSpec spec;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestDbSpec::load()
{
    DbSpec spec;
    QVERIFY(spec.isLoaded() == false);

    // Test: Null xml schema file + Null spec file
    QVERIFY(spec.load("", "") == false);
    QVERIFY(spec.isLoaded() == false);

    // Test: Null xml schema file + empty spec file
    QVERIFY(spec.load("", "files/empty.xml") == false);
    QVERIFY(spec.isLoaded() == false);

    // Test: Null xml schema + invalid xml document
    QVERIFY(spec.load("", "files/invalid_instance.xml") == false);
    QVERIFY(spec.isLoaded() == false);

    // Test: Null xml schema + valid XML document
    QVERIFY(spec.load("", "files/valid_instance.xml") == false);
    QVERIFY(spec.isLoaded() == false);

    // Test: valid XML schema + empty stream
    QVERIFY(spec.load("db_schema-0.2.xsd", "files/empty.xml") == false);
    QVERIFY(spec.isLoaded() == false);

    // Test: valid XML schema + invalid XML document
    QVERIFY(spec.load("db_schema-0.2.xsd", "files/invalid_instance.xml") == false);
    QVERIFY(spec.isLoaded() == false);

    // Test: valid XML schema + valid XML instance
    QVERIFY(spec.load("db_schema-0.2.xsd", "files/valid_instance.xml"));
    QVERIFY(spec.isLoaded());

    // Test: valid XML schema + invalid XML document - the spec should be reset
    QVERIFY(spec.load("db_schema-0.2.xsd", "files/invalid_instance.xml") == false);
    QVERIFY(spec.isLoaded() == false);
}

void TestDbSpec::setMessageHandler()
{
    DbSpec spec;
    MessageHandler messageHandler;

    spec.setMessageHandler(&messageHandler);
    QVERIFY(spec.load("db_schema-0.2.xsd", "files/invalid_instance.xml") == false);
    QVERIFY(spec.isLoaded() == false);
    QVERIFY(messageHandler.line() == 3);
    QVERIFY(messageHandler.column() == 111);
}

void TestDbSpec::ddl()
{
    DbSpec spec;
    QVERIFY(spec.load("db_schema-0.2.xsd", "files/valid_instance.xml"));
    QVERIFY(spec.isLoaded());

    // Verify some of the content
    QDomElement root = spec.ddl();
    QVERIFY(root.tagName() == "db_schema");

    QDomElement firstChild = root.firstChildElement();
    QVERIFY(firstChild.tagName() == "version");
    QVERIFY(firstChild.text() == "0.1");

    QDomElement secondChild = firstChild.nextSiblingElement();
    QVERIFY(secondChild.tagName() == "table");
    QVERIFY(secondChild.hasAttribute("name"));
    QVERIFY(secondChild.attribute("name") == "data_tree");
}

// Missing tests: duplicate index signatures (two different indexes on the exact same fields)
void TestDbSpec::isLogicallyValid()
{
    DbSpec spec;

    // Test: before any xml instance is loaded, it should return false
    QVERIFY(spec.isLogicallyValid() == false);

    MessageHandler messageHandler;
    spec.setMessageHandler(&messageHandler);

    // ----------------
    // Logical error test cases (all files should validate, but throw logical errors)
    QStringList files;
    files << "le_dup_table_names.xml";  // Test: duplicate table names
    files << "le_dup_field_names.xml";  // Test: duplicate field names
    files << "le_dup_index_names.xml";  // Test: duplicate index names

    files << "le_auto_increment_text.xml";     // Test: auto increment on text
    files << "le_auto_increment_real.xml";     // Test: auto increment on real
    files << "le_auto_increment_blob.xml";     // Test: auto increment on blob

    files << "le_multi_auto_increment.xml";                 // Test: multiple auto increment integer fields
    files << "le_auto_increment_not_primary_key.xml";       // Test: auto_increment which is not declared as the primary key
    files << "le_auto_increment_partial_primary_key.xml";   // Test: auto_increment which is part of a primary key
    files << "le_auto_increment_without_primary_key.xml";   // Test: auto_increment field in table that does not contain a primary key

    files << "le_dup_primary_key.xml";         // Test: duplicate primary key fields
    files << "le_invalid_primary_key.xml";     // Test: primary key does not reference field in table

    files << "le_dup_src_foreign_key.xml";     // Test: duplicate foreign src field
    files << "le_dup_ref_foreign_key.xml";     // Test: duplicate foreign ref field
    files << "le_invalid_src_foreign_key.xml";       // Test: foreign src field does not exist
    files << "le_invalid_reftable_foreign_key.xml";  // Test: foreign ref table does not exist
    files << "le_invalid_ref_foreign_key.xml";       // Test: foreign ref field does not exist
    files << "le_unequal_src_ref_foreign_key.xml";   // Test: number foreign src fields != number of foreign ref fields

    files << "le_dup_index_fields.xml";        // Test: duplicate index fields
    files << "le_invalid_index_field.xml";     // Test: indexed field does not exist in table

    files << "le_dup_unique_key.xml";          // Test: duplicate unique key field
    files << "le_invalid_unique_key.xml";      // Test: unique key field which does not exist

    for (int i=0, z=files.count(); i<z; ++i)
    {
        QString file = "files/" + files.at(i);
        QVERIFY2(QFile::exists(file), QString("File '%1' does not exist").arg(file).toAscii());

        QVERIFY2(spec.load("db_schema-0.2.xsd", file), file.toAscii());
        QVERIFY(spec.isLoaded());
        QVERIFY2(spec.isLogicallyValid() == false, QString("File: %1, Message: %2").arg(files.at(i)).arg(messageHandler.statusMessage()).toAscii());
        QVERIFY(messageHandler.statusMessage().isEmpty() == false);
    }

    // -----------------
    // Success test cases - full SQL XML file

    // While valid, valid_instance.xml also contains a foreign key reference to itself, which is also important to test for
    QVERIFY(spec.load("db_schema-0.2.xsd", "files/valid_instance.xml"));
    QVERIFY(spec.isLoaded());
    QVERIFY2(spec.isLogicallyValid(), QString("%1").arg(messageHandler.statusMessage()).toAscii());
    QVERIFY(messageHandler.statusMessage().isEmpty());

    QVERIFY(spec.load("db_schema-0.2.xsd", "files/long_valid_instance.xml"));
    QVERIFY(spec.isLoaded());
    QVERIFY2(spec.isLogicallyValid(), QString("%1").arg(messageHandler.statusMessage()).toAscii());
    QVERIFY(messageHandler.statusMessage().isEmpty());
}

void TestDbSpec::sql()
{
    DbSpec spec;
    QVERIFY(spec.load("db_schema-0.2.xsd", "files/long_valid_instance.xml"));
    QVERIFY(spec.isLoaded());
    spec.sql();
}

QTEST_MAIN(TestDbSpec)
#include "TestDbSpec.moc"
