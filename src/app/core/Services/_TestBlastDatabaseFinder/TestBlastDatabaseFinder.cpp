/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QFile>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include "../BlastDatabaseFinder.h"
#include "../../metatypes.h"

class TestBlastDatabaseFinder : public QObject
{
    Q_OBJECT

public:
    TestBlastDatabaseFinder()
    {
        qRegisterMetaType<BlastDatabaseMetaPodVector>("BlastDatabaseMetaPodVector");
    }

private slots:
    void findBlastDatabases_throw();
    void findBlastDatabases_data();
    void findBlastDatabases();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestBlastDatabaseFinder::findBlastDatabases_throw()
{
    BlastDatabaseFinder x;

    // Test: no blastdbcmd program defined
    try
    {
        x.findBlastDatabases(1, "files");
        QVERIFY(0);
    }
    catch(...)
    {
        QVERIFY(1);
    }

    // Test: dummy program that we then delete
    QFile file("dummy");
    QVERIFY(!file.exists());
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write("Some dummy data");
    file.close();
    file.setPermissions(QFile::ReadOwner | QFile::ExeOwner);

    try
    {
        x.setBlastDbCmdPath("dummy");
        QVERIFY(1);
    }
    catch (...)
    {
        QVERIFY(0);
    }

    QVERIFY(QFile::remove("dummy"));
    QCOMPARE(x.blastDbCmdPath(), QString("dummy"));

    try
    {
        x.findBlastDatabases(2, "files");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }
}

void TestBlastDatabaseFinder::findBlastDatabases_data()
{
    QTest::addColumn<bool>("error");
    QTest::addColumn<int>("id");
    QTest::addColumn<QString>("path");
    QTest::addColumn<BlastDatabaseMetaPodVector>("podVector");

    QTest::newRow("empty directory") << false << 2 << "files/emptydir" << BlastDatabaseMetaPodVector();
    QTest::newRow("non-existent directory") << false << -23 << "files/missing_directory" << BlastDatabaseMetaPodVector();
    QTest::newRow("mixed directory but no blast databases") << false << 3 << "files" << BlastDatabaseMetaPodVector();

    BlastDatabaseMetaPodVector podVector;

    // Alias file
    podVector << BlastDatabaseMetaPod();
    podVector.last().title_ = "Alias File Test";
    podVector.last().file_ = QDir("files/blastdbs").canonicalPath() + "/Alias";
    podVector.last().isProtein_ = eTrue;
    podVector.last().nLetters_ = 4367;
    podVector.last().nSequences_ = 19;
    podVector.last().nBytes_ = 7534;

    // Gene file with no seq ids
    podVector << BlastDatabaseMetaPod();
    podVector.last().title_ = "saur_genes-noseqids";
    podVector.last().file_ = QDir("files/blastdbs/saur_genes-noseqids").canonicalPath();
    podVector.last().isProtein_ = eFalse;
    podVector.last().nLetters_ = 14703;
    podVector.last().nSequences_ = 20;
    podVector.last().nBytes_ = 6775;

    // Protein
    podVector << BlastDatabaseMetaPod();
    podVector.last().title_ = "saur_prots";
    podVector.last().file_ = QDir("files/blastdbs/saur_prots").canonicalPath();
    podVector.last().isProtein_ = eTrue;
    podVector.last().nLetters_ = 4881;
    podVector.last().nSequences_ = 20;
    podVector.last().nBytes_ = 10479;

    // Protein
    podVector << BlastDatabaseMetaPod();
    podVector.last().title_ = "Funky title *!@#$%%^&*())";
    podVector.last().file_ = QDir("files/blastdbs/saur_prots-noseqids").canonicalPath();
    podVector.last().isProtein_ = eTrue;
    podVector.last().nLetters_ = 4881;
    podVector.last().nSequences_ = 20;
    podVector.last().nBytes_ = 7910;

    // Nucleotide without original fasta file that made the database
    QTest::newRow("many databases") << false << 1023 << "files/blastdbs" << podVector;
}

void TestBlastDatabaseFinder::findBlastDatabases()
{
    QFETCH(bool, error);
    QFETCH(int, id);
    QFETCH(QString, path);
    QFETCH(BlastDatabaseMetaPodVector, podVector);

    BlastDatabaseFinder x;
    try
    {
        x.setBlastDbCmdPath("./blastdbcmd");

        QSignalSpy spyError(&x, SIGNAL(error(int,QString)));
        QSignalSpy spyFoundBlastDatabases(&x, SIGNAL(foundBlastDatabases(int,QString,BlastDatabaseMetaPodVector)));

        QEventLoop eventLoop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&x, SIGNAL(error(int,QString)), &eventLoop, SLOT(quit()));
        connect(&x, SIGNAL(finished(int,QByteArray)), &eventLoop, SLOT(quit()));
        connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

        x.findBlastDatabases(id, path);
        timer.start(30000);  // 30 s timeout
        eventLoop.exec();
        QVERIFY(timer.isActive());
        timer.stop();
        QVERIFY(!x.isRunning());    // Process should have finished by this time

        if (!error)
        {
            if (spyError.size() > 0)
                qDebug() << spyError.at(0);

            QCOMPARE(spyError.size(), 0);
            QCOMPARE(spyFoundBlastDatabases.size(), 1);
            QVariantList spyArguments = spyFoundBlastDatabases.takeFirst();
            QCOMPARE(spyArguments.at(0).toInt(), id);
            QCOMPARE(spyArguments.at(1).toString(), path);
            QCOMPARE(qvariant_cast<BlastDatabaseMetaPodVector>(spyArguments.at(2)), podVector);
        }
        else
        {
            QCOMPARE(spyError.size(), 1);
            QVariantList spyArguments = spyError.takeFirst();
            QCOMPARE(spyArguments.at(0).toInt(), id);
            QCOMPARE(spyFoundBlastDatabases.size(), 0);
        }
    }
    catch(...)
    {
        QVERIFY(0);
    }
}


QTEST_MAIN(TestBlastDatabaseFinder)
#include "TestBlastDatabaseFinder.moc"
