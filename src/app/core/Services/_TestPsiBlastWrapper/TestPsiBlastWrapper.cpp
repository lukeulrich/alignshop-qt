/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtTest/QtTest>

#include "../PsiBlastWrapper.h"
#include "../../BioString.h"
#include "../../PODs/PsiBlastConfig.h"

static const QString g_BlastDb = "/lbinf/uniref50.sc";
static const QString g_PsiblastPath = "/binf/bin/psiblast";

class TestPsiBlastWrapper : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void setConfig();
    void psiblastThrows_data();
    void psiblastThrows();

    void psiblastInvalidDatabase();

    void psiblastKill();    // Also tests the isRunning method

    void psiblast();        // Valid psiblast test
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPsiBlastWrapper::constructor()
{
    PsiBlastWrapper x;

    QCOMPARE(x.config(), PsiBlastConfig());
    QVERIFY(x.isRunning() == false);
    QVERIFY(x.errorOutput().isEmpty());
    QVERIFY(x.output().isEmpty());
}

void TestPsiBlastWrapper::setConfig()
{
    PsiBlastWrapper x;

    PsiBlastConfig config;
    config.setThreads(3);
    config.setAgPssmFile("ag-pssm-file");

    x.setConfig(config);

    QCOMPARE(x.config(), config);
}

void TestPsiBlastWrapper::psiblastThrows_data()
{
    QTest::addColumn<QString>("psiblastPath");
    QTest::addColumn<QString>("blastDb");

    QTest::newRow("empty psiblast path and blastdb") << "" << "";
    QTest::newRow("empty psiblastPath but valid blastdb") << "" << g_BlastDb;
    QTest::newRow("valid psiblastPath but empty blastdb") << g_PsiblastPath << "";
}

void TestPsiBlastWrapper::psiblastThrows()
{
    QFETCH(QString, psiblastPath);
    QFETCH(QString, blastDb);

    PsiBlastWrapper x;

    PsiBlastConfig config;
    config.setPsiBlastPath(psiblastPath);
    config.setBlastDatabase(blastDb);

    // Test: don't specify either psiblast path, blast database, or biostring
    try
    {
        x.psiblast(10, BioString());
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }
}

void TestPsiBlastWrapper::psiblastInvalidDatabase()
{
    // Test: database file exists (to avoid psiblast wrapper error), but is not formatted
    QString unformattedBlastDb = "files/empty_blast_db";
    QVERIFY(QFile::exists(unformattedBlastDb));

    PsiBlastConfig config;
    config.setBlastDatabase(unformattedBlastDb);
    config.setPsiBlastPath(g_PsiblastPath);

    PsiBlastWrapper x(config);

    QSignalSpy spyError(&x, SIGNAL(error(int,QString)));
    QSignalSpy spyFinished(&x, SIGNAL(finished(int,QString)));

    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&x, SIGNAL(error(int,QString)), &eventLoop, SLOT(quit()));
    connect(&x, SIGNAL(finished(int,QString)), &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    try
    {
        x.psiblast(10, "MSSIRKTYVLKLYVAGNTPNSVRALRTLNHILETEFQGVYALKVIDVLKNPQLA");

        timer.start(2000);  // 2 s timeout
        eventLoop.exec();

        QVERIFY(timer.isActive());
        timer.stop();

        QVERIFY(spyFinished.isEmpty());
        QCOMPARE(spyError.size(), 1);
        QVariantList spyArguments = spyError.takeFirst();
        QCOMPARE(spyArguments.at(0).toInt(), 10);

        QCOMPARE(x.errorOutput().isEmpty(), false);
    }
    catch(...)
    {
        QVERIFY(0);
    }

    // Test: Invalid configuration (but "valid" PsiBlastConfig)
    spyError.clear();
    spyFinished.clear();
    config.setBlastDatabase(g_BlastDb);
    config.setAlignments(0);
    config.setDescriptions(0);
    x.setConfig(config);
    try
    {
        x.psiblast(10, "MSSIRKTYVLKLYVAGNTPNSVRALRTLNHILETEFQGVYALKVIDVLKNPQLA");

        timer.start(2000);  // 2 s timeout
        eventLoop.exec();

        QVERIFY(timer.isActive());
        timer.stop();

        QVERIFY(spyFinished.isEmpty());
        QCOMPARE(spyError.size(), 1);
        QVariantList spyArguments = spyError.takeFirst();
        QCOMPARE(spyArguments.at(0).toInt(), 10);

        QCOMPARE(x.errorOutput().isEmpty(), false);
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

void TestPsiBlastWrapper::psiblastKill()
{
    PsiBlastConfig config;
    config.setBlastDatabase(g_BlastDb);
    config.setPsiBlastPath(g_PsiblastPath);
    config.setThreads(1);

    PsiBlastWrapper x(config);

    QSignalSpy spyError(&x, SIGNAL(error(int,QString)));
    QSignalSpy spyFinished(&x, SIGNAL(finished(int,QString)));

    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&x, SIGNAL(error(int,QString)), &eventLoop, SLOT(quit()));
    connect(&x, SIGNAL(finished(int,QString)), &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    try
    {
        x.psiblast(10, "MSSIRKTYVLKLYVAGNTPNSVRALRTLNHILETEFQGVYALKVIDVLKNPQLA");
        timer.start(1000);  // 1 s timeout
        eventLoop.exec();
        QVERIFY(timer.isActive() == false); // Timeout
        QVERIFY(x.isRunning());
        x.kill();
        eventLoop.exec();

        QVERIFY(spyFinished.isEmpty());
        QCOMPARE(spyError.size(), 1);
        QVariantList spyArguments = spyError.takeFirst();
        QCOMPARE(spyArguments.at(0).toInt(), 10);

        QCOMPARE(x.isRunning(), false);
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

void TestPsiBlastWrapper::psiblast()
{
    PsiBlastConfig config;
    config.setBlastDatabase(g_BlastDb);
    config.setPsiBlastPath(g_PsiblastPath);
    config.setThreads(6);

    PsiBlastWrapper x(config);

    QSignalSpy spyError(&x, SIGNAL(error(int,QString)));
    QSignalSpy spyFinished(&x, SIGNAL(finished(int,QString)));

    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&x, SIGNAL(error(int,QString)), &eventLoop, SLOT(quit()));
    connect(&x, SIGNAL(finished(int,QString)), &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    try
    {
        x.psiblast(10, "MSSIRKTYVLKLYVAGNTPNSVRALRTLNHILETEFQGVYALKVIDVLKNPQLA");
        timer.start(60000);  // 1 min timeout
        eventLoop.exec();

        // No timeout
        QVERIFY(timer.isActive());
        timer.stop();
        QCOMPARE(x.isRunning(), false);

        QCOMPARE(spyError.size(), 0);
        QCOMPARE(spyFinished.size(), 1);
        QVariantList spyArguments = spyFinished.takeFirst();
        QCOMPARE(spyArguments.at(0).toInt(), 10);
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

QTEST_MAIN(TestPsiBlastWrapper)
#include "TestPsiBlastWrapper.moc"
