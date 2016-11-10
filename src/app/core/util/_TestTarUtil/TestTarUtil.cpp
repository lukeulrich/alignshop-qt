/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtTest/QtTest>

#include "../TarUtil.h"

class TestTarUtil : public QObject
{
    Q_OBJECT

private slots:
    // These tests are in a specific order such that if a prior one fails, the others will likely also fail since
    // they depend upon the prior functions for testing purposes.
    void header();
    void toRawPod();
    void validChecksum();
    void toPod();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestTarUtil::header()
{
    TarUtil x;

    // Test: should throw an exception if a file does not exist
    QVERIFY(!QFile::exists("missing"));
    try
    {
        x.header("missing");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // Test: named pipe should not work
    try
    {
        x.header("files/named_pipe");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

#ifdef Q_OS_LINUX
    // Test: special character device should not work
    try
    {
        x.header("/dev/zero");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // Test: block device
    try
    {
        x.header("/dev/sda1");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }
#endif

    // Test: valid file
    QVERIFY(QFile::exists("files/numbers"));
    TarHeaderPod pod = x.header("files/numbers");
    QCOMPARE(pod.fileName_, QByteArray("files/numbers"));
    QCOMPARE(pod.permissions_, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
    QCOMPARE(pod.uid_, (uint)1000);
    QCOMPARE(pod.uid_, pod.gid_);
    QCOMPARE(pod.fileSize_, qint64(11));    // Bytes
    QCOMPARE(pod.lastModificationTime_, QDateTime::fromString("Tue Oct 4 10:12:40 2011"));
    QCOMPARE(pod.linkType_, eFileType);
    QVERIFY(pod.linkName_.isEmpty());

    QCOMPARE(pod.ustar_, true);
    QCOMPARE(pod.version_, 0);
    QCOMPARE(pod.userName_, QByteArray("ulrich"));
    QCOMPARE(pod.userName_, pod.groupName_);
    QCOMPARE(pod.devMajorNumber_, (uint)0);
    QCOMPARE(pod.devMinorNumber_, (uint)0);
    QVERIFY(pod.prefix_.isEmpty());

    // Test: directory without trailing slash
    QVERIFY(QFile::exists("files/dir1"));
    pod = x.header("files/dir1");
    QCOMPARE(pod.fileName_, QByteArray("files/dir1/"));
    QCOMPARE(pod.permissions_, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                               QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                               QFile::ReadGroup | QFile::ExeGroup |
                               QFile::ReadOther | QFile::ExeOther);
    QCOMPARE(pod.uid_, (uint)1000);
    QCOMPARE(pod.uid_, pod.gid_);
    QCOMPARE(pod.fileSize_, (qint64)0);
    QCOMPARE(pod.lastModificationTime_, QDateTime::fromString("Tue Oct 4 10:12:52 2011"));
    QCOMPARE(pod.linkType_, eDirectoryType);
    QVERIFY(pod.linkName_.isEmpty());

    QCOMPARE(pod.ustar_, true);
    QCOMPARE(pod.version_, 0);
    QCOMPARE(pod.userName_, QByteArray("ulrich"));
    QCOMPARE(pod.userName_, pod.groupName_);
    QCOMPARE(pod.devMajorNumber_, (uint)0);
    QCOMPARE(pod.devMinorNumber_, (uint)0);
    QVERIFY(pod.prefix_.isEmpty());

    // Test: directory with trailing slash
    QVERIFY(QFile::exists("files/dir1/"));
    pod = x.header("files/dir1/");
    QCOMPARE(pod.fileName_, QByteArray("files/dir1/"));
    QCOMPARE(pod.permissions_, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                               QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                               QFile::ReadGroup | QFile::ExeGroup |
                               QFile::ReadOther | QFile::ExeOther);
    QCOMPARE(pod.uid_, (uint)1000);
    QCOMPARE(pod.uid_, pod.gid_);
    QCOMPARE(pod.fileSize_, (qint64)0);    // Bytes
    QCOMPARE(pod.lastModificationTime_, QDateTime::fromString("Tue Oct 4 10:12:52 2011"));
    QCOMPARE(pod.linkType_, eDirectoryType);
    QVERIFY(pod.linkName_.isEmpty());

    QCOMPARE(pod.ustar_, true);
    QCOMPARE(pod.version_, 0);
    QCOMPARE(pod.userName_, QByteArray("ulrich"));
    QCOMPARE(pod.userName_, pod.groupName_);
    QCOMPARE(pod.devMajorNumber_, (uint)0);
    QCOMPARE(pod.devMinorNumber_, (uint)0);
    QVERIFY(pod.prefix_.isEmpty());

    // Test: symlink
    QVERIFY(QFile::exists("files/dir_link"));
    pod = x.header("files/dir_link");
    QCOMPARE(pod.fileName_, QByteArray("files/dir_link"));
    QCOMPARE(pod.permissions_, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                               QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                               QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                               QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
    QCOMPARE(pod.uid_, (uint)1000);
    QCOMPARE(pod.uid_, pod.gid_);
    QCOMPARE(pod.fileSize_, (qint64)0);    // Bytes
    QCOMPARE(pod.lastModificationTime_, QDateTime::fromString("Tue Oct 4 17:24:24 2011"));
    QCOMPARE(pod.linkType_, eSymLinkType);
    QCOMPARE(pod.linkName_, QByteArray("dir1"));

    QCOMPARE(pod.ustar_, true);
    QCOMPARE(pod.version_, 0);
    QCOMPARE(pod.userName_, QByteArray("ulrich"));
    QCOMPARE(pod.userName_, pod.groupName_);
    QCOMPARE(pod.devMajorNumber_, (uint)0);
    QCOMPARE(pod.devMinorNumber_, (uint)0);
    QVERIFY(pod.prefix_.isEmpty());
}

void TestTarUtil::toRawPod()
{
    TarUtil x;

    // Test: regular file
    {
        QVERIFY(QFile::exists("files/numbers"));
        TarHeaderPod pod = x.header("files/numbers");

        char expectRawPod[kTarHeaderSize] = {
            'f','i','l','e','s','/','n','u','m','b','e','r','s','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '0','0','0','0','6','0','0','\0','0','0','0','1','7','5','0','\0','0','0','0','1','7','5','0','\0','0',
            '0','0','0','0','0','0','0','0','1','3','\0','1','1','6','4','2','6','1','2','1','3','0','\0','0','1',
            '3','0','7','2','\0',' ','0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','u','s','t','a','r',' ',' ','\0','u','l','r','i','c','h','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','u','l','r',
            'i','c','h','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'
        };

        TarHeaderRawPod rawPod = x.toRawPod(pod);
        for (int i=0; i< kTarHeaderSize; ++i)
        {
            //qDebug() << i << (int)expectRawPod[i] << "<-->" << (int)rawPod.rawData_[i];
            QVERIFY2(expectRawPod[i] == rawPod.rawData_[i], QString("Index error: %1").arg(i).toAscii());
        }
    }

    // Test: directory
    {
        QVERIFY(QFile::exists("files/dir1"));
        TarHeaderPod pod = x.header("files/dir1");

        char expectRawPod[kTarHeaderSize] = {
            'f','i','l','e','s','/','d','i','r','1','/','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '0','0','0','0','7','5','5','\0','0','0','0','1','7','5','0','\0','0','0','0','1','7','5','0','\0','0',
            '0','0','0','0','0','0','0','0','0','0','\0','1','1','6','4','2','6','1','2','1','4','4','\0','0','1',
            '2','3','5','6','\0',' ','5','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','u','s','t','a','r',' ',' ','\0','u','l','r','i','c','h','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','u','l','r',
            'i','c','h','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'
        };

        TarHeaderRawPod rawPod = x.toRawPod(pod);
        for (int i=0; i< kTarHeaderSize; ++i)
        {
//            qDebug() << i << expectRawPod[i] << "<-->" << rawPod.rawData_[i];
            QVERIFY2(expectRawPod[i] == rawPod.rawData_[i], QString("Index error: %1").arg(i).toAscii());
        }
    }

    // Test: symlink
    {
        QVERIFY(QFile::exists("files/dir_link"));
        TarHeaderPod pod = x.header("files/dir_link");

        char expectRawPod[kTarHeaderSize] = {
            'f','i','l','e','s','/','d','i','r','_','l','i','n','k','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '0','0','0','0','7','7','7','\0','0','0','0','1','7','5','0','\0','0','0','0','1','7','5','0','\0','0',
            '0','0','0','0','0','0','0','0','0','0','\0','1','1','6','4','2','6','7','4','6','1','0','\0','0','1',
            '4','0','2','2','\0',' ','2','d','i','r','1','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','u','s','t','a','r',' ',' ','\0','u','l','r','i','c','h','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','u','l','r',
            'i','c','h','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'
        };

        TarHeaderRawPod rawPod = x.toRawPod(pod);
        for (int i=0; i< kTarHeaderSize; ++i)
        {
//            qDebug() << i << expectRawPod[i] << "<-->" << rawPod.rawData_[i];
            QVERIFY2(expectRawPod[i] == rawPod.rawData_[i], QString("Index error: %1").arg(i).toAscii());
        }
    }
}

void TestTarUtil::validChecksum()
{
    TarUtil x;

    // Test: empty pod
    TarHeaderRawPod rawPod;
    QVERIFY(x.validChecksum(rawPod) == false);

    // Test: regular file
    QVERIFY(QFile::exists("files/numbers"));
    TarHeaderPod pod = x.header("files/numbers");
    QVERIFY(x.validChecksum(x.toRawPod(pod)));

    // Test: directory
    QVERIFY(QFile::exists("files/dir1"));
    pod = x.header("files/dir1");
    QVERIFY(x.validChecksum(x.toRawPod(pod)));

    // Test: symlink
    QVERIFY(QFile::exists("files/dir_link"));
    pod = x.header("files/dir_link");
    QVERIFY(x.validChecksum(x.toRawPod(pod)));
}

void TestTarUtil::toPod()
{
    TarUtil x;

    QVERIFY(QFile::exists("files/numbers"));
    TarHeaderPod pod = x.header("files/numbers");
    TarHeaderRawPod rawPod = x.toRawPod(pod);
    QVERIFY(x.toPod(rawPod) == pod);

    QVERIFY(QFile::exists("files/dir1"));
    pod = x.header("files/dir1");
    rawPod = x.toRawPod(pod);
    QVERIFY(x.toPod(rawPod) == pod);

    QVERIFY(QFile::exists("files/dir_link"));
    pod = x.header("files/dir_link");
    rawPod = x.toRawPod(pod);
    QVERIFY(x.toPod(rawPod) == pod);
}

QTEST_APPLESS_MAIN(TestTarUtil)
#include "TestTarUtil.moc"
