/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../../PODs/PsiBlastConfig.h"
#include "../../PODs/Q3Prediction.h"
#include "../../metatypes.h"
#include "../PsiBlastStructureTool.h"

static const QString g_BlastDb = "/lbinf/uniref50.sc";
static const QString g_PsiblastPath = "/binf/bin/psiblast";
static const QString nnFile1 = "../../../resources/data/nn-sec-stage1.net";
static const QString nnFile2 = "../../../resources/data/nn-sec-stage2.net";

class TestPsiBlastStructureTool : public QObject
{
    Q_OBJECT

public:
    TestPsiBlastStructureTool()
    {
        qRegisterMetaType<Q3Prediction>("Q3Prediction");
    }

private slots:
    void predictSecondary();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPsiBlastStructureTool::predictSecondary()
{
    PsiBlastConfig config;
    config.setIterations(2);
    config.setThreads(QThread::idealThreadCount());
    config.setBlastDatabase(g_BlastDb);
    config.setPsiBlastPath(g_PsiblastPath);

    // Test: invalid psiblast config (e.g. bad database)
    PsiBlastStructureTool x(config, nnFile1, nnFile2);
    QSignalSpy spyCanceled(&x, SIGNAL(canceled(int)));
    QSignalSpy spyError(&x, SIGNAL(error(int,QString)));
    QSignalSpy spyFinished(&x, SIGNAL(finished(int,Q3Prediction)));

    config.setBlastDatabase("");
    x.setPsiBlastConfig(config);
    x.predictSecondary(1, "ABCDEF");
    QVERIFY(spyFinished.isEmpty());
    QCOMPARE(spyError.size(), 1);
    QVERIFY(!x.isRunning());
    spyError.clear();

    // Cannot test invalid neural network file because it will throw an assertion

    // Test: Cancel action
    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&x, SIGNAL(canceled(int)), &eventLoop, SLOT(quit()));
    connect(&x, SIGNAL(error(int,QString)), &eventLoop, SLOT(quit()));
    connect(&x, SIGNAL(finished(int,Q3Prediction)), &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    config.setBlastDatabase(g_BlastDb);
    x.setPsiBlastConfig(config);
    x.predictSecondary(3, "MSSIRKTYVLKLYVAGNTPNSVRALRTLNHILETEFQGVYALKVIDVLKNPQLA");
    timer.start(10); // 10 ms timeout
    eventLoop.exec();
    QVERIFY(timer.isActive() == false);
    QVERIFY(x.isRunning());
    x.cancel();
    eventLoop.exec();

    QCOMPARE(spyCanceled.size(), 1);
    QCOMPARE(spyCanceled.takeFirst().at(0).toInt(), 3);
    QVERIFY(spyError.isEmpty());
    QVERIFY(spyFinished.isEmpty());

    // Test: valid search
    x.predictSecondary(3, "MSSIRKTYVLKLYVAGNTPNSVRALRTLNHILETEFQGVYALKVIDVLKNPQLA");
    timer.start(60000); // 60 s timeout
    eventLoop.exec();
    QVERIFY(timer.isActive());
    timer.stop();
    QVERIFY(!x.isRunning());
    QVERIFY(spyCanceled.isEmpty());
    QVERIFY(spyError.isEmpty());
    QCOMPARE(spyFinished.size(), 1);
    QVariantList spyArguments = spyFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(qvariant_cast<Q3Prediction>(spyArguments.at(1)).isEmpty(), false);
    spyFinished.clear();

    // Test: valid search without any hits
    // Inordinately high evalue cutoff to prevent any hits above the threshold
    QSignalSpy spyProgressChanged(&x, SIGNAL(progressChanged(int,int)));
    config.setEvalue(1e-100);
    x.setPsiBlastConfig(config);
    x.predictSecondary(3, "MSSIRKTYVLKLYVAGNTPNSVRALRTLNHILETEFQGVYALKVIDVLKNPQLA");
    timer.start(60000); // 60 s timeout
    eventLoop.exec();
    QVERIFY(timer.isActive());
    timer.stop();
    QVERIFY(!x.isRunning());
    QVERIFY(spyCanceled.isEmpty());
    QVERIFY(spyError.isEmpty());
    QCOMPARE(spyFinished.size(), 1);
    spyArguments = spyFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    // Key test condition
    QCOMPARE(qvariant_cast<Q3Prediction>(spyArguments.at(1)).isEmpty(), true);

    // Normally, the final progress changed value would be equivalent to the number of sequences in the database;
    // however, in the case when no significant hits are found, it simply emits 1, 1 to indicate it is done.
    QVERIFY(spyProgressChanged.size() > 0);
    spyArguments = spyProgressChanged.takeLast();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
}

QTEST_MAIN(TestPsiBlastStructureTool)
#include "TestPsiBlastStructureTool.moc"
