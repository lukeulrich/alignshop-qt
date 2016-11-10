/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../AbstractAnonSeq.h"
#include "../AminoSeq.h"

class MockAbstractAnonSeq : public AbstractAnonSeq
{
public:
    MockAbstractAnonSeq(int id, const Seq &seq) : AbstractAnonSeq(id, seq)
    {
    }
};

class TestAminoSeq : public QObject
{
    Q_OBJECT

private slots:
    void test1();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAminoSeq::test1()
{
    AminoSeq a(1, 2, 3, "Amino", "Seq", 0);
}


QTEST_APPLESS_MAIN(TestAminoSeq)
#include "TestAminoSeq.moc"
