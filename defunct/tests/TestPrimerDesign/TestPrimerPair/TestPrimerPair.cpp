/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScopedPointer>
#include "PrimerDesign/primer.h"
#include "PrimerDesign/primerpair.h"

using namespace PrimerDesign;

class TestPrimerPair : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Getters/setters
    void testForward();
    void testReverse();
    void testHeteroScore();
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
void TestPrimerPair::constructor()
{
    PrimerPair p;
    QVERIFY(p.heteroScore() == 0);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Getters/setters
void TestPrimerPair::testForward()
{
    PrimerPair p;
    Primer primer;
    primer.setSequence(constants::kDnaCharacters);
    QVERIFY(p.setForwardPrimer(primer));
    QVERIFY(p.forwardPrimer().sequence() == constants::kDnaCharacters);
}

void TestPrimerPair::testReverse()
{
    PrimerPair p;
    Primer primer;
    primer.setSequence(constants::kDnaCharacters);
    QVERIFY(p.setOppositePrimer(primer));
    QVERIFY(p.oppositePrimer().sequence() == constants::kDnaCharacters);
}

void TestPrimerPair::testHeteroScore()
{
    PrimerPair p;
    QVERIFY(p.setHeteroScore(2));
    QVERIFY(!p.setHeteroScore(-1));
    QVERIFY(p.heteroScore() == 2);
}

QTEST_MAIN(TestPrimerPair)
#include "TestPrimerPair.moc"
