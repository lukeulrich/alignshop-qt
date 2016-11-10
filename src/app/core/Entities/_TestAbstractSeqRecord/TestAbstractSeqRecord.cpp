/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../AbstractSeqRecord.h"
#include "../AbstractAnonSeq.h"

class MockAbstractAnonSeq : public AbstractAnonSeq
{
public:
    MockAbstractAnonSeq(int id, const Seq &seq) : AbstractAnonSeq(id, seq)
    {
    }
};

class MockAbstractSeqRecord : public AbstractSeqRecord
{
public:
    MockAbstractSeqRecord(int id, int start, int stop, const QString &name, const QString &description, MockAbstractAnonSeq *swine) :
        AbstractSeqRecord(id, start, stop, name, description), swine_(swine)
    {

    }

    MockAbstractAnonSeq *anonSeq() const
    {
        return swine_;
    }

private:
    MockAbstractAnonSeq *swine_;
};

class TestAbstractSeqRecord : public QObject
{
    Q_OBJECT

private slots:
    void test1();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAbstractSeqRecord::test1()
{
    MockAbstractSeqRecord r(1, 1, 4, "bob", "marley", 0);
}


QTEST_APPLESS_MAIN(TestAbstractSeqRecord)
#include "TestAbstractSeqRecord.moc"
