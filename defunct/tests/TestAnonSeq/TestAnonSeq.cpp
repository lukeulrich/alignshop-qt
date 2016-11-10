/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QDebug>

#include "AnonSeq.h"

#include "AminoString.h"
#include "DnaString.h"
#include "RnaString.h"

class TestAnonSeq : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorEmpty();
    void constructorBasic();
    void constructorPointer();
    void constructorAminoString();
    void constructorDnaString();
    void constructorRnaString();
    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    void assignOther();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void id();
    void isValid();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestAnonSeq::constructorEmpty()
{
    AnonSeq aseq();
}

void TestAnonSeq::constructorBasic()
{
    BioString bs("ABC---def---123");

    AnonSeq aseq(1, bs);

    // Double check that the biostring is properly normalized, masked, and ungapped
    QVERIFY(aseq.bioString().sequence() == "ABCDEFXXX");

    // Confirm that the id is correct
    QVERIFY(aseq.id() == 1);
}

void TestAnonSeq::constructorPointer()
{
    AnonSeq a(999, new BioString("ABC--"));  // Takes ownership of b, so we do not delete it here

    QVERIFY(a.bioString().sequence() == "ABC");
}

void TestAnonSeq::constructorAminoString()
{
    AminoString astr("ABC---def---123");

    AnonSeq aseq(1, astr);

    // Double check that the biostring is properly normalized, masked, and ungapped
    QVERIFY(aseq.bioString().sequence() == "ABCDEFXXX");

    // Confirm that the id is correct
    QVERIFY(aseq.id() == 1);
}

void TestAnonSeq::constructorDnaString()
{
    DnaString dstr("ABC---def...123");

    AnonSeq aseq(1, dstr);

    // Double check that the biostring is properly normalized, masked, and ungapped
    QVERIFY(aseq.bioString().sequence() == "ABCDNNNNN");

    // Confirm that the id is correct
    QVERIFY(aseq.id() == 1);
}

void TestAnonSeq::constructorRnaString()
{
    RnaString rstr("ABC---def...123");
    AnonSeq aseq(1, rstr);

    // Double check that the biostring is properly normalized, masked, and ungapped
    QVERIFY(aseq.bioString().sequence() == "ABCDNNNNN");

    // Confirm that the id is correct
    QVERIFY(aseq.id() == 1);
}

void TestAnonSeq::constructorCopy()
{
    BioString bs("ABC");

    AnonSeq aseq1(1, bs);
    AnonSeq aseq2(aseq1);

    QVERIFY(aseq2.bioString().sequence() == "ABC");
    QVERIFY(aseq2.id() == 1);
}

// ------------------------------------------------------------------------------------------------
// Assignment operators
void TestAnonSeq::assignOther()
{
    BioString bs1("ABC");
    BioString bs2("DEF");

    AnonSeq aseq1(1, bs1);
    AnonSeq aseq2(2, bs2);

    aseq1 = aseq2;

    QVERIFY(aseq1.bioString().sequence() == "DEF");
    QVERIFY(aseq2.bioString().sequence() == "DEF");
    QVERIFY(aseq1.id() == 2);
    QVERIFY(aseq2.id() == 2);
}

// ------------------------------------------------------------------------------------------------
// Public methods
void TestAnonSeq::id()
{
    BioString bs("ABC");

    // There is no limit on the range of internal ids
    for (int i=-100; i< 100; i += 5)
    {
        AnonSeq aseq(i, bs);
        QVERIFY(aseq.id() == i);
    }
}

void TestAnonSeq::isValid()
{
    AnonSeq aseq;
    QVERIFY(aseq.isValid() == false);

    AnonSeq aseq2(0, BioString("blah"));
    QVERIFY(aseq2.isValid());

    AnonSeq aseq3(34, BioString(""));
    QVERIFY(aseq3.isValid() == false);
}

QTEST_MAIN(TestAnonSeq)
#include "TestAnonSeq.moc"


