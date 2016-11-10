/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../BlastDbCmdBase.h"

class TestBlastDbCmdBase : public QObject
{
    Q_OBJECT

private slots:
    // Also tests the path equivalent version of this method
    void setBlastDbCmdPath();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestBlastDbCmdBase::setBlastDbCmdPath()
{
    BlastDbCmdBase x;

    QVERIFY(x.blastDbCmdPath().isEmpty());
    QVERIFY(!QFile::exists("missing"));

    // Test: missing path
    try
    {
        x.setBlastDbCmdPath("missing");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // Test: directory
    try
    {
        QVERIFY(QFile::exists("files"));
        x.setBlastDbCmdPath("files");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // Test: non-executable file
    try
    {
        QVERIFY(QFile::exists("TestBlastDbCmdBase.pro"));
        QVERIFY(!(QFile::permissions("TestBlastDbCmdBase.pro") & QFile::ExeOwner));
        x.setBlastDbCmdPath("TestBlastDbCmdBase.pro");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // Test: executable file of size 0
    try
    {
        QVERIFY(QFile::exists("files/emptyfile"));
        QVERIFY(QFile::permissions("files/emptyfile") & QFile::ExeOwner);
        QVERIFY(QFileInfo("files/emptyfile").size() == 0);
        x.setBlastDbCmdPath("files/emptyfile");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // Test: valid path
    QVERIFY(QFile::exists("blastdbcmd"));
    try
    {
        x.setBlastDbCmdPath("blastdbcmd");
        QCOMPARE(x.blastDbCmdPath(), QString("blastdbcmd"));
    }
    catch (QString &)
    {
        QVERIFY(0);
    }
}


QTEST_APPLESS_MAIN(TestBlastDbCmdBase)
#include "TestBlastDbCmdBase.moc"
