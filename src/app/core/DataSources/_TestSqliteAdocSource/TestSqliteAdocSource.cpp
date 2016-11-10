#include <QtCore/QCryptographicHash>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtSql/QSqlError>

#include "../SqliteAdocSource.h"
#include "../../AdocTreeNode.h"

class TestSqliteAdocSource : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void createAndOpen();
    void createAndOpenMemory();
    void astringCrud();
    void aminoSeqCrud();
    void dstringCrud();
    void dnaSeqCrud();

    void readEntityTree();
    void saveEntityTree();
};

void TestSqliteAdocSource::createAndOpen()
{
    QString fileName = "bob.db";
    QFile::remove(fileName);

    SqliteAdocSource source;
    QVERIFY(source.fileName().isEmpty());
    QVERIFY(source.createAndOpen(fileName));
    QVERIFY(source.isOpen());
    QCOMPARE(source.fileName(), fileName);
    QVERIFY(QFile::exists(fileName));

    source.close();
    QVERIFY(source.isOpen() == false);
    QVERIFY(source.fileName().isEmpty());

    // Test: now attempt to open it
    QVERIFY(source.open(fileName));
    QVERIFY(source.isOpen());
    QCOMPARE(source.fileName(), fileName);
    source.close();
    QVERIFY(source.fileName().isEmpty());

    QFile::remove(fileName);
}

void TestSqliteAdocSource::createAndOpenMemory()
{
    SqliteAdocSource source;
    QVERIFY(source.fileName().isEmpty());
    QVERIFY(source.createAndOpen(":memory:"));
    QVERIFY(source.isOpen());
    QCOMPARE(source.fileName(), QString(":memory:"));

    source.close();
    QVERIFY(source.isOpen() == false);
    QVERIFY(source.fileName().isEmpty());
}

void TestSqliteAdocSource::astringCrud()
{
    SqliteAdocSource source;
    QVERIFY(source.astringCrud());
}

void TestSqliteAdocSource::aminoSeqCrud()
{
    SqliteAdocSource source;
    QVERIFY(source.aminoSeqCrud());
}

void TestSqliteAdocSource::dstringCrud()
{
    SqliteAdocSource source;
    QVERIFY(source.dstringCrud());
}

void TestSqliteAdocSource::dnaSeqCrud()
{
    SqliteAdocSource source;
    QVERIFY(source.dnaSeqCrud());
}

void TestSqliteAdocSource::readEntityTree()
{
    QString fileName = "bob.db";
    QFile::remove(fileName);

    SqliteAdocSource source;
    QVERIFY(source.createAndOpen(fileName));
    QVERIFY(source.isOpen());

    AdocTreeNode *root = source.readEntityTree();
    QVERIFY(root != nullptr);
    QCOMPARE(root->childCount(), 0);
    delete root;
    root = nullptr;

    // Test: insert some data and see how it loads
    QSqlQuery query(source.database());
    if (!query.prepare("INSERT INTO entity_tree (type_id, type, entity_id, label, lft, rgt) "
                       "VALUES (?, ?, ?, ?, ?, ?)"))
    {
        qDebug() << query.lastError().text();
        QVERIFY(0);
    }

    query.bindValue(0, eRootNode);
    query.bindValue(1, "Root");
    query.bindValue(2, QVariant(QVariant::Int));
    query.bindValue(3, "Root");
    query.bindValue(4, 1);
    query.bindValue(5, 6);
    QVERIFY(query.exec());

    query.bindValue(0, eGroupNode);
    query.bindValue(1, "Group");
    query.bindValue(2, QVariant(QVariant::Int));
    query.bindValue(3, "PAS domains");
    query.bindValue(4, 2);
    query.bindValue(5, 3);
    QVERIFY(query.exec());

    query.bindValue(0, eGroupNode);
    query.bindValue(1, "Group");
    query.bindValue(2, QVariant(QVariant::Int));
    query.bindValue(3, "ChIP-Seq");
    query.bindValue(4, 4);
    query.bindValue(5, 5);
    QVERIFY(query.exec());

    root = source.readEntityTree();
    QCOMPARE(root->childCount(), 2);
    QCOMPARE(root->childAt(0)->nodeType_, eGroupNode);
    QCOMPARE(root->childAt(0)->entityId(), 0);
    QCOMPARE(root->childAt(0)->label_, QString("PAS domains"));
    QCOMPARE(root->childAt(0)->childCount(), 0);

    QCOMPARE(root->childAt(1)->nodeType_, eGroupNode);
    QCOMPARE(root->childAt(1)->entityId(), 0);
    QCOMPARE(root->childAt(1)->label_, QString("ChIP-Seq"));
    QCOMPARE(root->childAt(1)->childCount(), 0);
    delete root;
    root = nullptr;

    source.close();
}

void TestSqliteAdocSource::saveEntityTree()
{
    QString fileName = "bob.db";
    QFile::remove(fileName);

    SqliteAdocSource source;
    QVERIFY(source.createAndOpen(fileName));
    QVERIFY(source.isOpen());

    // Test: make sure that all prior records are erased
    QSqlQuery insert(source.database());
    if (!insert.prepare("INSERT INTO entity_tree (type_id, type, entity_id, label, lft, rgt) "
                       "VALUES (?, ?, ?, ?, ?, ?)"))
    {
        qDebug() << insert.lastError().text();
        QVERIFY(0);
    }

    insert.bindValue(0, eGroupNode);
    insert.bindValue(1, "Dummy");
    insert.bindValue(2, QVariant(QVariant::Int));
    insert.bindValue(3, "Dummy node");
    insert.bindValue(4, 1);
    insert.bindValue(5, 6);
    QVERIFY(insert.exec());

    source.saveEntityTree(nullptr);
    QSqlQuery query(source.database());
    QVERIFY(query.exec("SELECT count(*) FROM entity_tree"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    // Test: repeat the above, but this time with a real tree
    insert.bindValue(0, eGroupNode);
    insert.bindValue(1, "Dummy");
    insert.bindValue(2, QVariant(QVariant::Int));
    insert.bindValue(3, "Dummy node");
    insert.bindValue(4, 1);
    insert.bindValue(5, 6);
    QVERIFY(insert.exec());

    AdocTreeNode *root = new AdocTreeNode(eRootNode, "Root");
    root->appendChild(new AdocTreeNode(eGroupNode, "PAS domains"));
    root->appendChild(new AdocTreeNode(eGroupNode, "ChIP-Seq"));

    try
    {
        source.saveEntityTree(root);
    }
    catch (...)
    {
        QVERIFY(0);
    }

    QVERIFY(query.exec("SELECT count(*) FROM entity_tree"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 3);

    // Test: re-read the entries to make sure they were saved properly
    AdocTreeNode *root2 = source.readEntityTree();
    QVERIFY(root != root2);
    QCOMPARE(root2->childCount(), 2);
    QVERIFY((*root2->childAt(0)) == (*root->childAt(0)));
    QVERIFY((*root2->childAt(1)) == (*root->childAt(1)));

    delete root;
    delete root2;
    root = nullptr;
    root2 = nullptr;

    source.close();
}

QTEST_APPLESS_MAIN(TestSqliteAdocSource);

#include "TestSqliteAdocSource.moc"
