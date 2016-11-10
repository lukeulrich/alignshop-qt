/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../Adoc.h"
#include "../core/AdocNodeData.h"

class TestAdoc : public QObject
{
    Q_OBJECT

private slots:
    void test1();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAdoc::test1()
{
    Adoc adoc;
    QVERIFY(adoc.create());
    QVERIFY(adoc.isOpen());
    QVERIFY(adoc.isModified());
    QVERIFY(adoc.isTemporary());

    adoc.entityTree()->appendChild(new TreeNode<AdocNodeData>(AdocNodeData(eGroupNode, "LuxR domains")));
    adoc.saveAs("bobbies.db");

    QBENCHMARK
    {
        adoc.setModified(true);
        adoc.save();
    }

    QFile::remove("bobbies.db");
}

QTEST_APPLESS_MAIN(TestAdoc)
#include "TestAdoc.moc"
