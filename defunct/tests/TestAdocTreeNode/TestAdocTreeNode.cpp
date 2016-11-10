/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QScopedPointer>
#include <QtTest/QtTest>

//#include <QtSql/QSqlError>
//#include <QtSql/QSqlQuery>

#include "AdocTreeNode.h"

class TestAdocTreeNode : public QObject
{
    Q_OBJECT

public:
    TestAdocTreeNode();

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();             // Also test label() and nodeType() here
    void copyConstructor();

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    void childrenBetween();

    // ------------------------------------------------------------------------------------------------
    // Public methods
//    void eraseRecord();

    void isNodeContainer();

    // ------------------------------------------------------------------------------------------------
    // Static methods
    void isContainer();
    void nodeTypeEnum();
    void nodeTypeString();

private:
    QStringList enumStrings_;
    QList<AdocTreeNode::NodeType> nodeTypes_;

    // Define those nodes that are containers
    QHash<AdocTreeNode::NodeType, bool> containers_;
};


TestAdocTreeNode::TestAdocTreeNode() : QObject()
{
    enumStrings_ << "root";
    nodeTypes_ << AdocTreeNode::RootType;

    enumStrings_ << "group";
    nodeTypes_ << AdocTreeNode::GroupType;

    enumStrings_ << "seqamino";
    nodeTypes_ << AdocTreeNode::SeqAminoType;

    enumStrings_ << "seqdna";
    nodeTypes_ << AdocTreeNode::SeqDnaType;

    enumStrings_ << "seqrna";
    nodeTypes_ << AdocTreeNode::SeqRnaType;

    enumStrings_ << "subseqamino";
    nodeTypes_ << AdocTreeNode::SubseqAminoType;

    enumStrings_ << "subseqdna";
    nodeTypes_ << AdocTreeNode::SubseqDnaType;

    enumStrings_ << "subseqrna";
    nodeTypes_ << AdocTreeNode::SubseqRnaType;

    enumStrings_ << "msaamino";
    nodeTypes_ << AdocTreeNode::MsaAminoType;

    enumStrings_ << "msadna";
    nodeTypes_ << AdocTreeNode::MsaDnaType;

    enumStrings_ << "msarna";
    nodeTypes_ << AdocTreeNode::MsaRnaType;

    enumStrings_ << "primer";
    nodeTypes_ << AdocTreeNode::PrimerType;

    enumStrings_ << "undefined";
    nodeTypes_ << AdocTreeNode::UndefinedType;

    containers_[AdocTreeNode::RootType] = true;
    containers_[AdocTreeNode::GroupType] = true;
    containers_[AdocTreeNode::MsaAminoType] = true;
    containers_[AdocTreeNode::MsaDnaType] = true;
    containers_[AdocTreeNode::MsaRnaType] = true;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
void TestAdocTreeNode::constructor()
{
    // Test: basic constructor
    QScopedPointer<AdocTreeNode> root(new AdocTreeNode(AdocTreeNode::RootType));

    QVERIFY(root->parent() == 0);
    QVERIFY(root->childCount() == 0);
    QVERIFY(root->label_.isEmpty());
    QVERIFY(root->nodeType_ == AdocTreeNode::RootType);

    // Test: basic constructor with label
    root.reset(new AdocTreeNode(AdocTreeNode::RootType, "ROOT LABEL"));
    QVERIFY(root->label_ == "ROOT LABEL");

    // Test: basic constructor with label
    QScopedPointer<AdocTreeNode> node(new AdocTreeNode(AdocTreeNode::SeqAminoType, "12345"));
    QVERIFY(node->label_ == "12345");

    // Test: basic constructor with label and fkId
    node.reset(new AdocTreeNode(AdocTreeNode::SeqAminoType, "12345", 12345));
    QVERIFY(node->label_ == "12345");
    QVERIFY(node->fkId_ == 12345);
}


void TestAdocTreeNode::copyConstructor()
{
    AdocTreeNode *root = new AdocTreeNode(AdocTreeNode::RootType, "Root", 50);
    AdocTreeNode *child1 = new AdocTreeNode(AdocTreeNode::GroupType, "Group", 100);
    root->appendChild(child1);

    AdocTreeNode *rootCopy = new AdocTreeNode(*root);
    QCOMPARE(rootCopy->nodeType_, root->nodeType_);
    QCOMPARE(rootCopy->label_, root->label_);
    QCOMPARE(rootCopy->fkId_, root->fkId_);
    QVERIFY(rootCopy->parent() == 0);
    QVERIFY(rootCopy->childCount() == 0);

    AdocTreeNode *childCopy = new AdocTreeNode(*child1);
    QCOMPARE(childCopy->nodeType_, child1->nodeType_);
    QCOMPARE(childCopy->label_, child1->label_);
    QCOMPARE(childCopy->fkId_, child1->fkId_);
    QVERIFY(childCopy->parent() == 0);
    QVERIFY(childCopy->childCount() == 0);

    delete root;
    root = 0;
    child1 = 0;
    delete rootCopy;
    rootCopy = 0;
    delete childCopy;
    childCopy = 0;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
void TestAdocTreeNode::childrenBetween()
{
    AdocTreeNode *root = new AdocTreeNode();
    AdocTreeNode *child = new AdocTreeNode();
    AdocTreeNode *child2 = new AdocTreeNode();
    AdocTreeNode *child3 = new AdocTreeNode();

    root->appendChild(child);
    root->appendChild(child2);
    root->appendChild(child3);

    QList<AdocTreeNode *> result;
    result = root->childrenBetween(0, 0);
    QCOMPARE(result.count(), 1);
    QCOMPARE(result.at(0), child);

    result = root->childrenBetween(1, 2);
    QCOMPARE(result.count(), 2);
    QCOMPARE(result.at(0), child2);
    QCOMPARE(result.at(1), child3);

    delete root;
    root = 0;
    child = 0;
    child2 = 0;
    child3 = 0;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods

/*
void TestAdocTreeNode::eraseRecord()
{
    // -----------------------
    // Suite: invalid database
    AdocTreeNode root(AdocTreeNode::RootType, "Root");

    // Test: fkId_ = 0 and empty foreignTable_
    QVERIFY(root.fkId() == 0);
    QVERIFY(root.eraseRecord(QSqlDatabase()) == false);

    // Test: fkId_ = 0, foreignTable = projects
    root.setForeignTable("projects");
    QVERIFY(root.eraseRecord(QSqlDatabase()) == false);

    // Test: fkId_ = 3, empty foreignTable_
    root.setForeignTable(QString());
    root.setFkId(3);
    QVERIFY(root.eraseRecord(QSqlDatabase()) == false);

    // Test: fkId_ = 3, foreignTable_ = projects
    root.setForeignTable("projects");
    QVERIFY(root.eraseRecord(QSqlDatabase()) == false);

    // ---------------------
    // Suite: valid database
    AdocTreeNode project(AdocTreeNode::ProjectType, "Sample project");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
    db.setDatabaseName(":memory:");
    db.open();
    QVERIFY(db.exec("CREATE TABLE projects (id integer primary key autoincrement, name text not null, created text)").lastError().type() == QSqlError::NoError);

    // Test: fkId_ = 0, empty foreignTable - should return true and nothing should happen
    QVERIFY(project.fkId() == 0);
    QVERIFY(project.eraseRecord(db));

    // Test: fkId_ = 0, foreignTable = "projects"
    project.setForeignTable("projects");
    QVERIFY(project.eraseRecord(db) == false);

    // Test: fkId_ = 3, empty foreignTable
    project.setForeignTable(QString());
    project.setFkId(3);
    QVERIFY(project.eraseRecord(db) == false);

    // Test: fkId_ = 3, non-existent foreignTable
    project.setForeignTable("missing");
    QVERIFY(project.eraseRecord(db) == false);

    // Test: fkId_ pointing to non-existent record, foreignTable = projects; should still work.
    project.setForeignTable("projects");
    QVERIFY(project.eraseRecord(db));

    // Test: erase single existing record
    QVERIFY(db.exec("insert into projects (id, name) values (1, 'Sample project')").lastError().type() == QSqlError::NoError);
    project.setFkId(1);
    QVERIFY(project.eraseRecord(db));
    QSqlQuery query = db.exec("SELECT count(*) FROM projects");
    QVERIFY(query.next() && query.value(0).toInt() == 0);
    QVERIFY(project.fkId() == 1);

    // --------------------------------------------------------
    // Suite: eraseRecord and descendant entities
    //
    // Test tree:
    //
    // base
    // |___group
    QScopedPointer<AdocTreeNode> base(new AdocTreeNode(AdocTreeNode::ProjectType, "Sample project"));
    AdocTreeNode *group = new AdocTreeNode(AdocTreeNode::GroupType, "Sample group");
    base->appendChild(group);

    // Insert a row for base
    QVERIFY(db.exec("insert into projects (id, name) values (1, 'Sample project')").lastError().type() == QSqlError::NoError);
    base->setForeignTable("projects");
    base->setFkId(1);

    // Create the groups table
    QVERIFY(db.exec("CREATE TABLE groups (id integer primary key autoincrement, name text not null)").lastError().type() == QSqlError::NoError);

    // -------------------------------------------------
    // Sub-suite: descendant node with invalid configuration
    // Test: group fkId_ = 10, but empty group foreignTable_
    group->setFkId(10);
    QVERIFY(base->eraseRecord(db) == false);
    query = db.exec("SELECT count(*) from projects");
    QVERIFY(query.next() && query.value(0).toInt() == 1);   // Ensure that the record is still there

    // Test: group fkId_ = 0, but non-empty group foreignTable_
    group->setFkId(0);
    group->setForeignTable("groups");
    QVERIFY(base->eraseRecord(db) == false);
    query = db.exec("SELECT count(*) from projects");
    QVERIFY(query.next() && query.value(0).toInt() == 1);   // Ensure that the record is still there

    // Test: group fkId_ = 1, foreignTable_ = "missing"
    group->setFkId(1);
    group->setForeignTable("missing");
    QVERIFY(base->eraseRecord(db) == false);
    query = db.exec("SELECT count(*) from projects");
    QVERIFY(query.next() && query.value(0).toInt() == 1);   // Ensure that the record is still there

    // Test: group fkId_ = 1, foreignTable_ = "groups"
    group->setForeignTable("groups");
    QVERIFY(base->eraseRecord(db));
    query = db.exec("SELECT count(*) from projects");
    QVERIFY(query.next() && query.value(0).toInt() == 0);   // Ensure that the project record was deleted

    // -------------------------------------------------
    // Sub-suite: descendant node without corresponding table as indicated by a fkId_ of 0 and an empty foreignTable_
    // Test: group fkId_ = 0, group empty foreignTable_
    QVERIFY(db.exec("insert into projects (id, name) values (1, 'Sample project')").lastError().type() == QSqlError::NoError);
    group->setForeignTable(QString());
    group->setFkId(0);
    QVERIFY(base->eraseRecord(db));
    query = db.exec("SELECT count(*) from projects");
    QVERIFY(query.next() && query.value(0).toInt() == 0);   // Ensure that the project record was deleted

    // -------------------------------------------------
    // Sub-suite: descendant node with corresponding table
    QVERIFY(db.exec("insert into projects (id, name) values (1, 'Sample project')").lastError().type() == QSqlError::NoError);
    QVERIFY(db.exec("insert into groups (id, name) values (1, 'Sample group')").lastError().type() == QSqlError::NoError);

    // Test: proper fkId_ and foreignTable for both base and group
    base->setFkId(1);
    group->setForeignTable("groups");
    group->setFkId(1);

    QVERIFY(base->eraseRecord(db));
    query = db.exec("SELECT count(*) from projects");
    QVERIFY(query.next() && query.value(0).toInt() == 0);   // Ensure that the project record was deleted
    query = db.exec("SELECT count(*) from groups");
    QVERIFY(query.next() && query.value(0).toInt() == 0);   // Ensure that the project record was deleted

    // -------------------------------------------------
    // Sub-suite: descendant node without corresponding table which has descendant node with corresponding table
    //
    // Test tree:
    //
    // o Alpha (projects.id = 1)
    // |------ o Child1 (no table)
    //         |------- o Child 1.1 (groups.id = 1)
    QVERIFY(db.exec("insert into projects (id, name) values (1, 'Sample project')").lastError().type() == QSqlError::NoError);
    QVERIFY(db.exec("insert into groups (id, name) values (1, 'Sample group')").lastError().type() == QSqlError::NoError);

    QScopedPointer<AdocTreeNode> node(new AdocTreeNode(AdocTreeNode::ProjectType, "Alpha", "projects", 1));
    AdocTreeNode *child1 = new AdocTreeNode(AdocTreeNode::GroupType, "Child1");
    node->appendChild(child1);
    child1->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Child1.1", "groups", 1));

    QVERIFY(node->eraseRecord(db));
    query = db.exec("SELECT count(*) from projects");
    QVERIFY(query.next() && query.value(0).toInt() == 0);   // Ensure that the project record was deleted
    query = db.exec("SELECT count(*) from groups");
    QVERIFY(query.next() && query.value(0).toInt() == 0);   // Ensure that the project record was deleted
}
*/

void TestAdocTreeNode::isNodeContainer()
{
    // Iterative over all possible node types and check whether they classify themselves properly
    for (int i=0, z= AdocTreeNode::MaxNodeType; i< z; ++i)
    {
        AdocTreeNode::NodeType type = static_cast<AdocTreeNode::NodeType>(i);
        AdocTreeNode node(type);
        if (containers_.contains(type))
            QVERIFY(node.isContainer());
        else
            QVERIFY(node.isContainer() == false);
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static methods
void TestAdocTreeNode::isContainer()
{
    // Iterative over all possible node types and check whether they classify themselves properly
    for (int i=0, z= AdocTreeNode::MaxNodeType; i< z; ++i)
    {
        AdocTreeNode::NodeType type = static_cast<AdocTreeNode::NodeType>(i);
        QVERIFY(AdocTreeNode::isContainer(type) == containers_.contains(type));
    }
}

void TestAdocTreeNode::nodeTypeEnum()
{
    for (int i=0, z=enumStrings_.count(); i<z; ++i)
        QVERIFY(AdocTreeNode::nodeTypeEnum(enumStrings_.at(i)) == nodeTypes_.at(i));
}

void TestAdocTreeNode::nodeTypeString()
{
    for (int i=0, z=nodeTypes_.count(); i<z; ++i)
        QVERIFY(AdocTreeNode::nodeTypeString(nodeTypes_.at(i)) == enumStrings_.at(i));
}

QTEST_MAIN(TestAdocTreeNode)
#include "TestAdocTreeNode.moc"
